#include "../../AxiomEditor.h"
#include "../AudioBackend.h"

#ifdef PORTAUDIO
#include <portaudio.h>
#endif

using namespace AxiomBackend;

class StandaloneAudioBackend : public AudioBackend {
public:
    NumValue **outputPortal = nullptr;

    void handleConfigurationChange(const AudioConfiguration &configuration) override {
        // we only care about the first number output portal
        outputPortal = nullptr;
        for (size_t i = 0; i < configuration.portals.size(); i++) {
            const auto &portal = configuration.portals[i];
            if (portal.type == PortalType::OUTPUT && portal.value == PortalValue::AUDIO) {
                outputPortal = getAudioPortal(i);
                break;
            }
        }
    }

    AudioConfiguration createDefaultConfiguration() override {
        return AudioConfiguration({ConfigurationPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
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
            /*std::cout << "Processing " << (endProcessPos - processPos) << " samples" << std::endl;
            if (backend->outputPortal) {
                auto outputNum = **backend->outputPortal;
                std::cout << "Output = " << outputNum.left << " " << outputNum.right << " (" << (uint8_t) outputNum.form
            << ")" << std::endl;

                if (outputNum.left != 0) {
                    qt_noop();
                }
            }*/

            for (auto i = processPos; i < endProcessPos; i++) {
                backend->generate();

                if (backend->outputPortal) {
                    auto outputNum = **backend->outputPortal;
                    *outputNums++ = outputNum.left;
                    *outputNums++ = outputNum.right;
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
    StandaloneAudioBackend backend;
    AxiomEditor editor(&backend);
    backend.startupAudio();
    auto returnVal = editor.run();
    backend.shutdownAudio();
    return returnVal;
}
