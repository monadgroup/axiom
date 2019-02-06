#include <iostream>
#include <string>

#include "../../AxiomApplication.h"
#include "../../AxiomEditor.h"
#include "editor/backend/EventConverter.h"
#include "../AudioBackend.h"

#ifdef PORTAUDIO
#include <portaudio.h>
#endif

#ifdef PORTMIDI
#include <portmidi.h>
#endif

using namespace AxiomBackend;

static constexpr int MIDI_BUFFER_SIZE = 32;

class StandaloneAudioBackend : public AudioBackend {
public:
    ssize_t midiInputPortal = -1;
    ssize_t audioOutputPortal = -1;
    ssize_t midiOutputPortal = -1;
    NumValue **outputPortal = nullptr;
    MidiValue **outputMidi = nullptr;

    void handleConfigurationChange(const AudioConfiguration &configuration) override {
        // we only care about the first MIDI input and first number output portal
        midiInputPortal = -1;
        audioOutputPortal = -1;
        midiOutputPortal = -1;
        outputPortal = nullptr;
        outputMidi = nullptr;
        for (size_t i = 0; i < configuration.portals.size(); i++) {
            const auto &portal = configuration.portals[i];
            if (outputPortal == nullptr && portal.type == PortalType::OUTPUT && portal.value == PortalValue::AUDIO) {
                audioOutputPortal = i;
                outputPortal = getAudioPortal(i);
            } else if (midiInputPortal == -1 && portal.type == PortalType::INPUT && portal.value == PortalValue::MIDI) {
                midiInputPortal = (ssize_t) i;
            } else if (outputMidi == nullptr && portal.type == PortalType::OUTPUT && portal.value == PortalValue::MIDI) {
                midiOutputPortal = (ssize_t) i;
                outputMidi = getMidiPortal(i);
            }

            if (outputPortal != nullptr && midiInputPortal != -1 && midiOutputPortal != -1) {
                break;
            }
        }
    }

    DefaultConfiguration createDefaultConfiguration() override {
        return DefaultConfiguration({DefaultPortal(PortalType::OUTPUT, PortalValue::AUDIO, "Speakers")});
    }

    bool doesSaveInternally() const override { return false; }

    std::string getPortalLabel(size_t portalIndex) const override {
        if ((ssize_t) portalIndex == midiInputPortal || (ssize_t) portalIndex == audioOutputPortal || (ssize_t) portalIndex == midiOutputPortal) {
            return "1";
        }
        return "?";
    }

    void previewEvent(AxiomBackend::MidiEvent event) override {
        if (midiInputPortal == -1) return;
        queueMidiEvent(0, (size_t) midiInputPortal, event);
    }

#ifdef PORTMIDI
    PmStream *midiInputStream = nullptr;
    PmStream *midiOutputStream = nullptr;

    static void displayAvailableDevices() {
        std::cout << "Available MIDI devices:" << std::endl;
        auto availableDeviceCount = Pm_CountDevices();
        for (auto i = 0; i < availableDeviceCount; i++) {
            auto deviceInfo = Pm_GetDeviceInfo(i);
            std::cout << "  " << i << "  " << deviceInfo->name << " (";
            if (deviceInfo->input) {
                std::cout << "input";
            }
            if (deviceInfo->input && deviceInfo->output) {
                std::cout << " ";
            }
            if (deviceInfo->output) {
                std::cout << "output";
            }
            std::cout << ")" << std::endl;
        }
    }

    void openMidiInputStream(int inputMidiDevice) {
        if (inputMidiDevice == -2) {
            return;
        }
        if (inputMidiDevice == -1) {
            inputMidiDevice = Pm_GetDefaultInputDeviceID();
        }
        if (inputMidiDevice != pmNoDevice) {
            auto deviceInfo = Pm_GetDeviceInfo(inputMidiDevice);
            std::cout << "Using MIDI input: " << inputMidiDevice << " " << deviceInfo->name << std::endl;

            checkPmError(Pm_OpenInput(&midiInputStream,
                                      inputMidiDevice,
                                      nullptr,
                                      MIDI_BUFFER_SIZE,
                                      nullptr,
                                      nullptr));
        } else {
            std::cerr << "Not using MIDI input as no devices could be detected." << std::endl;
        }
    }

    void openMidiOutputStream(int outputMidiDevice) {
        if (outputMidiDevice == -2) {
            return;
        }
        if (outputMidiDevice == -1) {
            outputMidiDevice = Pm_GetDefaultOutputDeviceID();
        }
        if (outputMidiDevice != pmNoDevice) {
            auto deviceInfo = Pm_GetDeviceInfo(outputMidiDevice);
            std::cout << "Using MIDI output: " << outputMidiDevice << " " << deviceInfo->name << std::endl;

            checkPmError(Pm_OpenOutput(&midiOutputStream,
                                       outputMidiDevice,
                                       nullptr,
                                       MIDI_BUFFER_SIZE,
                                       nullptr,
                                       nullptr,
                    // MIDI event latency: it's currently 0 since we don't know the audio latency, there might be
                    // some way to fix this however?
                                       0));
        } else {
            std::cerr << "Not using MIDI output as no devices could be detected." << std::endl;
        }
    }

    static PmError checkPmError(PmError error) {
        if (error < 0) {
            std::cerr << "PortMidi error: " << Pm_GetErrorText(error) << std::endl;
            abort();
        }
        return error;
    }

    void processIncomingMidiEvents() {
        if (midiInputStream == nullptr) {
            return;
        }

        int numRead;
        do {
            PmEvent eventBuffer[MIDI_BUFFER_SIZE];
            numRead = Pm_Read(midiInputStream, eventBuffer, MIDI_BUFFER_SIZE);
            checkPmError((PmError) numRead);

            // Don't bother processing if there's nowhere to put the events
            if (midiInputPortal == -1) continue;

            for (auto eventIndex = 0; eventIndex < numRead; eventIndex++) {
                if (auto convertedEvent = convertFromMidi(eventBuffer[eventIndex].message)) {
                    queueMidiEvent(0, (size_t) midiInputPortal, *convertedEvent);
                }
            }
        } while (numRead >= MIDI_BUFFER_SIZE);
    }

    void processOutgoingMidiEvents() {
        if (midiOutputStream == nullptr || outputMidi == nullptr) {
            return;
        }

        // Read all queued events, convert them to MIDI and stuff them in the buffer
        auto midiQueue = *outputMidi;
        for (uint8_t i = 0; i < midiQueue->count; i++) {
            checkPmError(Pm_WriteShort(
                    midiOutputStream,
                    0,
                    convertToMidi(midiQueue->events[i])
            ));
        }
        midiQueue->count = 0;
    }
#endif

#ifdef PORTAUDIO
    PaStream *audioStream = nullptr;

    static void checkPaError(PaError error) {
        if (error != paNoError) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(error) << std::endl;
            abort();
        }
    }

    static int paCallback(const void *, void *outputBuffer, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
        auto backend = (StandaloneAudioBackend *) userData;
#ifdef PORTMIDI
        backend->processIncomingMidiEvents();
#endif

        uint64_t processPos = 0;

        auto outputNums = (float *) outputBuffer;

        auto sampleFrames64 = (uint64_t) framesPerBuffer;
        while (processPos < sampleFrames64) {
            auto context = backend->beginGenerate();
            auto endProcessPos = processPos + context.maxGenerateCount();
            if (endProcessPos > sampleFrames64) endProcessPos = sampleFrames64;

            for (auto i = processPos; i < endProcessPos; i++) {
                context.generate();

                if (backend->outputPortal) {
                    auto outputNum = **backend->outputPortal;
                    *outputNums++ = (float) outputNum.left;
                    *outputNums++ = (float) outputNum.right;
                }

#ifdef PORTMIDI
                backend->processOutgoingMidiEvents();
#endif

                if (backend->midiInputPortal != -1 && i == processPos) {
                    backend->clearMidi((size_t) backend->midiInputPortal);
                }
            }

            processPos = endProcessPos;
        }

        return 0;
    }
#endif

    void startupAudio(int inputMidiDevice, int outputMidiDevice) {
#ifdef PORTAUDIO
        checkPaError(Pa_Initialize());

        // todo: allow inputs and outputs to be customized (does PortAudio allow changing inputs and outputs
        // mid-stream?)
        checkPaError(Pa_OpenDefaultStream(&audioStream,
                                        0, // no inputs
                                        2, // stereo output
                                        paFloat32, 44100, paFramesPerBufferUnspecified, paCallback, this));
        checkPaError(Pa_StartStream(audioStream));
#endif

#ifdef PORTMIDI
        checkPmError(Pm_Initialize());
        displayAvailableDevices();
        openMidiInputStream(inputMidiDevice);
        openMidiOutputStream(outputMidiDevice);
#endif
    }

    void shutdownAudio() {
#ifdef PORTAUDIO
        checkPaError(Pa_StopStream(audioStream));
        checkPaError(Pa_CloseStream(audioStream));
        checkPaError(Pa_Terminate());
#endif

#ifdef PORTMIDI
        if (midiInputStream != nullptr) {
            checkPmError(Pm_Close(midiInputStream));
        }
        if (midiOutputStream != nullptr) {
            checkPmError(Pm_Close(midiOutputStream));
        }
        checkPmError(Pm_Terminate());
#endif
    }
};

int main(int argc, char *argv[]) {
    std::cout << "Starting application" << std::endl;
    AxiomApplication application;
    std::cout << "Starting backend" << std::endl;
    StandaloneAudioBackend backend;
    std::cout << "Starting editor" << std::endl;
    AxiomEditor editor(&application, &backend);

    int midiInputDevice = -1;
    int midiOutputDevice = -1;
    bool hasOpenedProject = false;

    for (auto i = 1; i < argc; i++) {
        // If --input or --output was provided, use their values for the MIDI input/output devices.
        // If there's a regular argument, load it as a project file.
        if (strcmp(argv[i], "--input") == 0) {
            i++;

            if (strcmp(argv[i], "none") == 0) {
                midiInputDevice = -2;
            } else {
                try {
                    midiInputDevice = (int) std::stoul(std::string(argv[i]), nullptr, 10);
                } catch (const std::invalid_argument &) {}
                catch (const std::out_of_range &) {}
            }
        } else if (strcmp(argv[i], "--output") == 0) {
            i++;

            if (strcmp(argv[i], "none") == 0) {
                midiOutputDevice = -2;
            } else {
                try {
                    midiOutputDevice = (int) std::stoul(std::string(argv[i]), nullptr, 10);
                } catch (const std::invalid_argument &) {}
                catch (const std::out_of_range &) {}
            }
        } else if (!hasOpenedProject) {
            editor.openProjectFile(argv[i]);
            hasOpenedProject = true;
        }
    }

    std::cout << "Starting audio" << std::endl;
    backend.startupAudio(midiInputDevice, midiOutputDevice);
    std::cout << "Opening editor" << std::endl;
    auto returnVal = editor.run();
    std::cout << "Shutting down audio" << std::endl;
    backend.shutdownAudio();
    std::cout << "Goodbye." << std::endl;
    return returnVal;
}
