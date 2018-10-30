#include <iostream>

#include "../../AxiomApplication.h"
#include "../../AxiomEditor.h"
#include "../AudioBackend.h"

#ifdef PORTAUDIO
#include <portaudio.h>
#endif

using namespace AxiomBackend;

class StandaloneAudioBackend : public AudioBackend {
public:
    ssize_t midiInputPortal = -1;
    ssize_t audioOutputPortal = -1;
    NumValue **outputPortal = nullptr;

    void handleConfigurationChange(const AudioConfiguration &configuration) override {
        // we only care about the first MIDI input and first number output portal
        midiInputPortal = -1;
        audioOutputPortal = -1;
        outputPortal = nullptr;
        for (size_t i = 0; i < configuration.portals.size(); i++) {
            const auto &portal = configuration.portals[i];
            if (outputPortal == nullptr && portal.type == PortalType::OUTPUT && portal.value == PortalValue::AUDIO) {
                audioOutputPortal = i;
                outputPortal = getAudioPortal(i);
            } else if (midiInputPortal == -1 && portal.type == PortalType::INPUT && portal.value == PortalValue::MIDI) {
                midiInputPortal = (ssize_t) i;
            }

            if (outputPortal != nullptr && midiInputPortal != -1) {
                break;
            }
        }
    }

    DefaultConfiguration createDefaultConfiguration() override {
        return DefaultConfiguration({DefaultPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
    }

    bool doesSaveInternally() const override { return false; }

    std::string getPortalLabel(size_t portalIndex) const override {
        if ((ssize_t) portalIndex == midiInputPortal || (ssize_t) portalIndex == audioOutputPortal) {
            return "1";
        }
        return "?";
    }

    void previewEvent(AxiomBackend::MidiEvent event) override {
        if (midiInputPortal == -1) return;
        auto lock = lockRuntime();
        queueMidiEvent(0, (size_t) midiInputPortal, event);
    }

#ifdef PORTAUDIO
    PaStream *stream = nullptr;

    static void checkError(PaError error) {
        if (error != paNoError) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;
            abort();
        }
    }

    static int paCallback(const void *, void *outputBuffer, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
        auto backend = (StandaloneAudioBackend *) userData;
        uint64_t processPos = 0;

        auto outputNums = (float *) outputBuffer;

        auto sampleFrames64 = (uint64_t) framesPerBuffer;
        while (processPos < sampleFrames64) {
            auto lock = backend->lockRuntime();
            auto sampleAmount = backend->beginGenerate();
            auto endProcessPos = processPos + sampleAmount;
            if (endProcessPos > sampleFrames64) endProcessPos = sampleFrames64;

            for (auto i = processPos; i < endProcessPos; i++) {
                backend->generate();

                if (backend->outputPortal) {
                    auto outputNum = **backend->outputPortal;
                    *outputNums++ = outputNum.left;
                    *outputNums++ = outputNum.right;
                }

                if (backend->midiInputPortal != -1 && i == processPos) {
                    backend->clearMidi((size_t) backend->midiInputPortal);
                }
            }

            processPos = endProcessPos;
        }

        return 0;
    }

    void startupAudio() {
        checkError(Pa_Initialize());

        // todo: allow inputs and outputs to be customized (does PortAudio allow changing inputs and outputs
        // mid-stream?)
        checkError(Pa_OpenDefaultStream(&stream,
                                        0, // no inputs
                                        2, // stereo output
                                        paFloat32, 44100, paFramesPerBufferUnspecified, paCallback, this));
        checkError(Pa_StartStream(stream));
    }

    void shutdownAudio() {
        checkError(Pa_StopStream(stream));
        checkError(Pa_CloseStream(stream));
        checkError(Pa_Terminate());
    }
#else
    void startupAudio() {}
    void shutdownAudio() {}
#endif
};

int main(int argc, char *argv[]) {
    std::cout << "Starting application" << std::endl;
    AxiomApplication application;
    std::cout << "Starting backend" << std::endl;
    StandaloneAudioBackend backend;
    std::cout << "Starting editor" << std::endl;
    AxiomEditor editor(&application, &backend);

    // if there's an argument provided, load it as a project file
    if (argc >= 2) {
        editor.openProjectFile(argv[1]);
    }

    std::cout << "Starting audio" << std::endl;
    backend.startupAudio();
    std::cout << "Opening editor" << std::endl;
    auto returnVal = editor.run();
    std::cout << "Shutting down audio" << std::endl;
    backend.shutdownAudio();
    std::cout << "Goodbye." << std::endl;
    return returnVal;
}
