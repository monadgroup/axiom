#include <iostream>
#include <thread>
#include <QtCore/QSharedMemory>
#include <QtCore/QTimer>

#include "VstChannel.h"
#include "Dispatcher.h"
#include "IdBuffer.h"
#include "../../AxiomApplication.h"
#include "../../AxiomEditor.h"
#include "../vst2-common/VstAdapter.h"
#include "../vst2-common/VstAudioBackend.h"
#include "editor/util.h"

using namespace AxiomBackend;

class BridgedAdapter : public VstAdapter {
public:
    VstChannel &channel;
    VstChannel::SeparateData sep;
    Dispatcher<VstChannel::GuiVstToAppQueue> guiDispatcher;
    Dispatcher<VstChannel::AudioVstToAppQueue> audioDispatcher;
    std::thread audioThread;
    std::thread guiDispatcherThread;
    QSharedMemory ioBuffer;
    VstAudioBackend *backend = nullptr;

    BridgedAdapter(VstChannel &channel, const std::string &id)
        : channel(channel),
          sep(id),
          guiDispatcher(channel.guiVstToApp, [this](const VstGuiMessage &message) {
              return dispatchGuiMessage(message);
          }),
          audioDispatcher(channel.audioVstToApp, [this](const VstAudioMessage &message) {
              return dispatchAudioMessage(message);
          }) {}

    void start(VstAudioBackend *backend) {
        this->backend = backend;

        audioThread = std::thread([this]() {
            std::cout << "Running audio dispatcher" << std::endl;
            audioDispatcher.run(sep.audioVstToAppData);
            std::cout << "Audio dispatcher has exited" << std::endl;
        });
        guiDispatcherThread = std::thread([this]() {
            std::cout << "Running GUI dispatcher" << std::endl;
            guiDispatcher.run(sep.guiVstToAppData);
            std::cout << "GUI dispatcher has exited" << std::endl;
        });
    }

    void adapterUpdateIo() override {
        // determine the size for a buffer that contains the specified IO
        auto allBuffersSize = IO_BUFFER_SIZE * (backend->audioInputs.size() + backend->audioOutputs.size()) * sizeof(float);

        AppGuiMessage msg(AppGuiMessageType::UPDATE_IO);
        msg.data.updateIo = {
            /* inputCount */ backend->audioInputs.size(),
            /* outputCount */ backend->audioOutputs.size(),
            /* newMemoryId */ 0
        };

        if (allBuffersSize > (size_t) ioBuffer.size()) {
            // allocate a new buffer
            msg.data.updateIo.newMemoryId = generateNewBufferId();
            ioBuffer.detach();
            ioBuffer.setKey(getBufferStringKey(msg.data.updateIo.newMemoryId));
            auto createSuccess = ioBuffer.create(allBuffersSize);
            assert(createSuccess);
        }

        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

    void adapterSetParameter(size_t parameter, AxiomBackend::NumValue value) override {
        AppGuiMessage msg(AppGuiMessageType::SET_PARAMETER);
        msg.data.setParameter = {
            (int) parameter, (float) value.left
        };
        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

private:
    void handleSetParameter(VstGuiSetParameterMessage message) {
        if ((size_t) message.index >= backend->automationInputs.size()) return;
        auto &param = backend->automationInputs[message.index];
        if (param) {
            auto val = *param->value;
            val->left = message.value;
            val->right = message.value;
            val->form = NumForm::CONTROL;
        }
    }

    void handleGetParameter(VstGuiGetParameterMessage message) {
        AppGuiMessage msg(AppGuiMessageType::PARAMETER_VALUE);
        msg.data.parameterValue = {0};

        if ((size_t) message.index < backend->automationInputs.size()) {
            auto &param = backend->automationInputs[message.index];
            if (param) {
                msg.data.parameterValue.value = (float) (*param->value)->left;
            }
        }

        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

    void handleGetParameterLabel(VstGuiGetParameterLabelMessage message) {
        AppGuiMessage msg(AppGuiMessageType::PARAMETER_LABEL);
        msg.data.parameterLabel.name[0] = 0;

        if ((size_t) message.index < backend->automationInputs.size()) {
            auto &param = backend->automationInputs[message.index];
            if (param) {
                auto val = *param->value;
                strncpy(msg.data.parameterLabel.name, AudioBackend::formatNumForm(val->left, val->form), 8);
            }
        }

        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

    void handleGetParameterDisplay(VstGuiGetParameterDisplayMessage message) {
        AppGuiMessage msg(AppGuiMessageType::PARAMETER_DISPLAY);
        msg.data.parameterDisplay.name[0] = 0;

        if ((size_t) message.index < backend->automationInputs.size()) {
            auto &param = backend->automationInputs[message.index];
            if (param) {
                auto val = *param->value;
                strncpy(msg.data.parameterDisplay.name, AudioBackend::formatNum(*val, false).c_str(), 8);
            }
        }

        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

    void handleGetParameterName(VstGuiGetParameterNameMessage message) {
        AppGuiMessage msg(AppGuiMessageType::PARAMETER_NAME);
        msg.data.parameterName.name[0] = 0;

        if ((size_t) message.index < backend->automationInputs.size()) {
            auto &param = backend->automationInputs[message.index];
            if (param) {
                strncpy(msg.data.parameterName.name, param->name.c_str(), 8);
            }
        }

        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

    void handleGetCanAutomateParameter(VstGuiGetCanAutomateParameterMessage message) {
        AppGuiMessage msg(AppGuiMessageType::CAN_AUTOMATE_PARAMETER);
        msg.data.canAutomateParameter.canAutomate = (size_t) message.index < backend->automationInputs.size();
        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

    void handleBeginSerialize(VstGuiBeginSerializeMessage message) {
        auto serializedBuffer = backend->serialize([this](QDataStream &stream) {
            backend->audioInputs.serialize(stream);
            backend->audioOutputs.serialize(stream);
            backend->automationInputs.serialize(stream);
        });

        // Create a shared memory buffer to copy the data across
        auto saveMemoryId = generateNewBufferId();
        QSharedMemory saveMemory(getBufferStringKey(saveMemoryId));
        auto createSuccess = saveMemory.create(serializedBuffer.size());
        assert(createSuccess);

        // Copy the data into it
        memcpy(saveMemory.data(), serializedBuffer.data(), (size_t) serializedBuffer.size());

        // Inform the VST that the data is available
        AppGuiMessage msg(AppGuiMessageType::FINISHED_SERIALIZE);
        msg.data.finishedSerialize.memoryId = saveMemoryId;
        msg.data.finishedSerialize.bufferSize = (uint64_t) serializedBuffer.size();
        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);

        // Wait for the app to respond so we know we can free the buffer
        while (true) {
            auto nextMsg = guiDispatcher.waitForNext(sep.guiVstToAppData);
            if (nextMsg.type == VstGuiMessageType::END_SERIALIZE) {
                break;
            }
        }
    }

    void handleBeginDeserialize(VstGuiBeginDeserializeMessage message) {
        // Load the shared memory that the data is in
        QSharedMemory loadMemory(getBufferStringKey(message.memoryId));
        auto attachSuccess = loadMemory.attach();
        assert(attachSuccess);

        auto byteArray = QByteArray::fromRawData((char *) loadMemory.data(), message.bufferSize);
        backend->deserialize(&byteArray, [this](QDataStream &stream, uint32_t version) {
            backend->audioInputs = NumParameters::deserialize(stream, version);
            backend->audioOutputs = NumParameters::deserialize(stream, version);
            backend->automationInputs = NumParameters::deserialize(stream, version);
        });

        // Inform the VST that we've finished loading
        AppGuiMessage msg(AppGuiMessageType::FINISHED_DESERIALIZE);
        channel.guiAppToVst.pushWhenAvailable(msg, sep.guiAppToVstData);
    }

    DispatcherHandlerResult dispatchGuiMessage(VstGuiMessage message) {
        // Queue the message to be processed on the GUI thread
        QMetaObject::invokeMethod(qApp, [this, message]() {
            switch (message.type) {
                case VstGuiMessageType::SET_PARAMETER:
                    handleSetParameter(message.data.setParameter);
                    break;
                case VstGuiMessageType::GET_PARAMETER:
                    handleGetParameter(message.data.getParameter);
                    break;
                case VstGuiMessageType::GET_PARAMETER_LABEL:
                    handleGetParameterLabel(message.data.getParameterLabel);
                    break;
                case VstGuiMessageType::GET_PARAMETER_DISPLAY:
                    handleGetParameterDisplay(message.data.getParameterDisplay);
                    break;
                case VstGuiMessageType::GET_PARAMETER_NAME:
                    handleGetParameterName(message.data.getParameterName);
                    break;
                case VstGuiMessageType::GET_CAN_AUTOMATE_PARAMETER:
                    handleGetCanAutomateParameter(message.data.getCanAutomateParameter);
                    break;
                case VstGuiMessageType::BEGIN_SERIALIZE:
                    handleBeginSerialize(message.data.beginSerialize);
                    break;
                case VstGuiMessageType::BEGIN_DESERIALIZE:
                    handleBeginDeserialize(message.data.beginDeserialize);
                    break;
                // Used as a response, no handling necessary
                case VstGuiMessageType::END_SERIALIZE:
                    break;
                default:
                    unreachable;
            }
        });

        return DispatcherHandlerResult::CONTINUE;
    }

    float *getInputBufferPtr(size_t inputIndex) {
        return reinterpret_cast<float *>(ioBuffer.data()) + inputIndex * IO_BUFFER_SIZE;
    }

    float *getOutputBufferPtr(size_t outputIndex) {
        return reinterpret_cast<float *>(ioBuffer.data()) + IO_BUFFER_SIZE * (backend->audioInputs.size() + outputIndex);
    }

    void handleGenerate(VstAudioGenerateMessage message) {
        auto sampleFrames64 = (uint64_t) message.sampleCount;
        uint64_t processPos = 0;
        while (processPos < sampleFrames64) {
            auto context = backend->beginGenerate();
            auto endProcessPos = processPos + context.maxGenerateCount();
            if (endProcessPos > sampleFrames64) endProcessPos = sampleFrames64;

            for (auto i = processPos; i < endProcessPos; i++) {
                for (size_t inputIndex = 0; inputIndex < backend->audioInputs.size(); inputIndex++) {
                    const auto &input = backend->audioInputs[inputIndex];
                    if (input) {
                        auto inputSource = getInputBufferPtr(inputIndex);
                        auto &inputNum = **input->value;
                        inputNum.left = inputSource[i * 2];
                        inputNum.right = inputSource[i * 2 + 1];
                        inputNum.form = AxiomBackend::NumForm::OSCILLATOR;
                    }
                }

                context.generate();

                for (size_t outputIndex = 0; outputIndex < backend->audioOutputs.size(); outputIndex++) {
                    const auto &output = backend->audioOutputs[outputIndex];
                    auto outputDest = getOutputBufferPtr(outputIndex);

                    if (output) {
                        auto outputNum = **output->value;
                        outputDest[i * 2] = (float) outputNum.left;
                        outputDest[i * 2 + 1] = (float) outputNum.right;
                    } else {
                        outputDest[i * 2] = 0;
                        outputDest[i * 2 + 1] = 0;
                    }
                }

                if (backend->midiInputPortal != -1 && i == processPos) {
                    backend->clearMidi((size_t) backend->midiInputPortal);
                }
            }

            processPos = endProcessPos;
        }

        AppAudioMessage msg(AppAudioMessageType::GENERATE_DONE);
        // todo: push or pushWhenAvailable here?
        channel.audioAppToVst.push(msg, sep.audioAppToVstData);
    }

    void handlePushMidiEvent(VstAudioPushMidiEventMessage message) {
        if (backend->midiInputPortal == -1) {
            return;
        }

        MidiEvent event;
        event.channel = message.channel;
        event.event = (MidiEventType) message.event;
        event.note = message.note;
        event.param = message.param;
        backend->queueMidiEvent((uint64_t) message.deltaSamples, (size_t) backend->midiInputPortal, event);
    }

    void handleSetSampleRate(VstAudioSetSampleRateMessage message) {
        backend->setSampleRate(message.sampleRate);
    }

    void handleSetBpm(VstAudioSetBpmMessage message) {
        backend->setBpm(message.bpm);
    }

    DispatcherHandlerResult dispatchAudioMessage(const VstAudioMessage &message) {
        switch (message.type) {
            case VstAudioMessageType::GENERATE:
                handleGenerate(message.data.generate);
                break;
            case VstAudioMessageType::PUSH_MIDI_EVENT:
                handlePushMidiEvent(message.data.pushMidiEvent);
                break;
            case VstAudioMessageType::SET_SAMPLE_RATE:
                handleSetSampleRate(message.data.setSampleRate);
                break;
            case VstAudioMessageType::SET_BPM:
                handleSetBpm(message.data.setBpm);
                break;
            default:
                unreachable;
        }

        return DispatcherHandlerResult::CONTINUE;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <shared id> <instrument/effect>" << std::endl;
        return 1;
    }

    std::cout << "Starting application" << std::endl;
    AxiomApplication application;

    std::cout << "Opening shared channel" << std::endl;
    QSharedMemory channelBuffer(argv[1]);
    if (!channelBuffer.attach()) {
        std::cerr << "Failed to open shared channel: " << channelBuffer.errorString().toStdString() << std::endl;
        return 1;
    }
    auto &channel = *reinterpret_cast<VstChannel *>(channelBuffer.data());

    std::cout << "Starting backend" << std::endl;
    BridgedAdapter adapter(channel, argv[1]);
    VstAudioBackend backend(adapter, strcmp(argv[2], "instrument") == 0);
    adapter.start(&backend);

    std::cout << "Starting editor" << std::endl;
    AxiomEditor editor(&application, &backend);

    return editor.run();
}
