#include "AxiomBridgedVstPlugin.h"

#include <public.sdk/source/vst2.x/aeffeditor.h>

#include "IdBuffer.h"
#include "editor/util.h"
#include "../AudioBackend.h"

class AxiomBridgedEditor : public AEffEditor {
public:
    explicit AxiomBridgedEditor(AxiomBridgedVstPlugin *plugin)
        : AEffEditor(plugin), plugin(plugin) {}

    void idle() override {
        plugin->guiDispatcher.idle();
    }

private:
    AxiomBridgedVstPlugin *plugin;
};

extern "C" {
AEffect *VSTPluginMain(audioMasterCallback audioMaster) {
    if (!audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0)) return nullptr;
    auto effect = new AxiomBridgedVstPlugin(audioMaster);
    return effect->getAeffect();
}
}

AxiomBackend::VstChannel &createChannel(QSharedMemory &mem) {
    auto createSuccess = mem.create(sizeof(AxiomBackend::VstChannel));
    assert(createSuccess);
    auto someVal = ::new (mem.data()) AxiomBackend::VstChannel;
    return *someVal;
}

AxiomBridgedVstPlugin::AxiomBridgedVstPlugin(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 255),
      channelMemKey(AxiomBackend::getBufferStringKey(AxiomBackend::generateNewBufferId())),
      channelMem(channelMemKey),
      channel(createChannel(channelMem)),
      guiDispatcher(channel.guiAppToVst, [this](const AxiomBackend::AppGuiMessage &message) {
          return dispatchGuiMessage(message);
      }),
      audioDispatcher(channel.audioAppToVst, [this](const AxiomBackend::AppAudioMessage &message) {
          return dispatchAudioMessage(message);
      }) {
    // todo: differentiate these slightly from the regular VST
#ifdef AXIOM_VST2_IS_SYNTH
    isSynth();
    setUniqueID(0x41584F53);
#else
    setUniqueID(0x41584F45);
#endif

    programsAreChunks();
    canProcessReplacing();

    setEditor(new AxiomBridgedEditor(this));

    QString appProcessPath = "axiom_vst2_bridge.exe";
    QStringList appProcessParams;
    appProcessParams.push_back(channelMemKey);
#ifdef AXIOM_VST2_IS_SYNTH
    appProcessParams.push_back("instrument");
#else
    appProcessParams.push_back("effect");
#endif
    appProcess.start(appProcessPath, appProcessParams);

    // Wait for the first IO update
    while (true) {
        auto nextGuiMessage = guiDispatcher.waitForNext();
        if (nextGuiMessage.type == AxiomBackend::AppGuiMessageType::UPDATE_IO) {
            break;
        }
    }
}

void AxiomBridgedVstPlugin::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    auto timeInfo = getTimeInfo(kVstTempoValid);
    if (timeInfo->flags & kVstTempoValid) {
        auto newBpm = (float) timeInfo->tempo;
        if (newBpm != currentBpm) {
            currentBpm = newBpm;
            AxiomBackend::VstAudioMessage msg(AxiomBackend::VstAudioMessageType::SET_BPM);
            msg.data.setBpm.bpm = newBpm;
            channel.audioVstToApp.push(msg);
        }
    }

    auto totalProcessed = 0;
    while (totalProcessed < sampleFrames) {
        auto processCount = std::min(sampleFrames - totalProcessed, (int) AxiomBackend::IO_SAMPLE_COUNT);

        // Copy the inputs into the shared memory
        for (size_t inputIndex = 0; inputIndex < expectedInputCount; inputIndex++) {
            auto inputDest = getInputBufferPtr(inputIndex);
            for (auto i = 0; i < processCount; i++) {
                inputDest[i * 2] = inputs[inputIndex * 2][totalProcessed + i];
                inputDest[i * 2 + 1] = inputs[inputIndex * 2 + 1][totalProcessed + i];
            }
        }

        // Tell the app to generate
        AxiomBackend::VstAudioMessage msg(AxiomBackend::VstAudioMessageType::GENERATE);
        msg.data.generate.sampleCount = (uint8_t) processCount;
        channel.audioVstToApp.push(msg);

        // Wait for the app to say it's finished
        while (audioDispatcher.waitForNext().type != AxiomBackend::AppAudioMessageType::GENERATE_DONE) {}

        // Copy the outputs back to the parameters
        for (size_t outputIndex = 0; outputIndex < expectedOutputCount; outputIndex++) {
            auto outputSrc = getOutputBufferPtr(outputIndex);
            for (auto i = 0; i < processCount; i++) {
                outputs[outputIndex * 2][totalProcessed + i] = outputSrc[i * 2];
                outputs[outputIndex * 2 + 1][totalProcessed + i] = outputSrc[i * 2 + 1];
            }
        }

        totalProcessed += processCount;
    }

    expectedInputCount = inputCount;
    expectedOutputCount = outputCount;
}

VstInt32 AxiomBridgedVstPlugin::processEvents(VstEvents *events) {
    // todo
    return 0;
}

void AxiomBridgedVstPlugin::setSampleRate(float sampleRate) {
    AxiomBackend::VstAudioMessage msg(AxiomBackend::VstAudioMessageType::SET_SAMPLE_RATE);
    msg.data.setSampleRate.sampleRate = sampleRate;
    channel.audioVstToApp.push(msg);
}

void AxiomBridgedVstPlugin::setParameter(VstInt32 index, float value) {
    AxiomBackend::VstGuiMessage msg(AxiomBackend::VstGuiMessageType::SET_PARAMETER);
    msg.data.setParameter.index = index;
    msg.data.setParameter.value = value;
    channel.guiVstToApp.pushWhenAvailable(msg);
}

float AxiomBridgedVstPlugin::getParameter(VstInt32 index) {
    AxiomBackend::VstGuiMessage msg(AxiomBackend::VstGuiMessageType::GET_PARAMETER);
    msg.data.getParameter.index = index;
    channel.guiVstToApp.pushWhenAvailable(msg);

    while (true) {
        auto nextMessage = guiDispatcher.waitForNext();
        if (nextMessage.type == AxiomBackend::AppGuiMessageType::PARAMETER_VALUE) {
            return nextMessage.data.parameterValue.value;
        }
    }
}

void AxiomBridgedVstPlugin::getParameterLabel(VstInt32 index, char *label) {
    AxiomBackend::VstGuiMessage msg(AxiomBackend::VstGuiMessageType::GET_PARAMETER_LABEL);
    msg.data.getParameterLabel.index = index;
    channel.guiVstToApp.pushWhenAvailable(msg);

    while (true) {
        auto nextMessage = guiDispatcher.waitForNext();
        if (nextMessage.type == AxiomBackend::AppGuiMessageType::PARAMETER_LABEL) {
            vst_strncpy(label, nextMessage.data.parameterLabel.name, 8);
            return;
        }
    }
}

void AxiomBridgedVstPlugin::getParameterDisplay(VstInt32 index, char *text) {
    AxiomBackend::VstGuiMessage msg(AxiomBackend::VstGuiMessageType::GET_PARAMETER_DISPLAY);
    msg.data.getParameterDisplay.index = index;
    channel.guiVstToApp.pushWhenAvailable(msg);

    while (true) {
        auto nextMessage = guiDispatcher.waitForNext();
        if (nextMessage.type == AxiomBackend::AppGuiMessageType::PARAMETER_DISPLAY) {
            vst_strncpy(text, nextMessage.data.parameterDisplay.name, 8);
            return;
        }
    }
}

void AxiomBridgedVstPlugin::getParameterName(VstInt32 index, char *text) {
    AxiomBackend::VstGuiMessage msg(AxiomBackend::VstGuiMessageType::GET_PARAMETER_NAME);
    msg.data.getParameterName.index = index;
    channel.guiVstToApp.pushWhenAvailable(msg);

    while (true) {
        auto nextMessage = guiDispatcher.waitForNext();
        if (nextMessage.type == AxiomBackend::AppGuiMessageType::PARAMETER_NAME) {
            vst_strncpy(text, nextMessage.data.parameterName.name, 8);
            return;
        }
    }
}

VstInt32 AxiomBridgedVstPlugin::getChunk(void **data, bool isPreset) {
    // todo
    return 0;
}

VstInt32 AxiomBridgedVstPlugin::setChunk(void *data, VstInt32 byteSize, bool isPreset) {
    // todo
    return 0;
}

bool AxiomBridgedVstPlugin::getEffectName(char *name) {
    vst_strncpy(name, AxiomBackend::PRODUCT_NAME, kVstMaxEffectNameLen);
    return true;
}

bool AxiomBridgedVstPlugin::getVendorString(char *text) {
    vst_strncpy(text, AxiomBackend::COMPANY_NAME, kVstMaxVendorStrLen);
    return true;
}

bool AxiomBridgedVstPlugin::getProductString(char *text) {
    vst_strncpy(text, AxiomBackend::FILE_DESCRIPTION, kVstMaxProductStrLen);
    return true;
}

VstInt32 AxiomBridgedVstPlugin::getVendorVersion() {
    return 1;
}

VstPlugCategory AxiomBridgedVstPlugin::getPlugCategory() {
    return kPlugCategSynth;
}

VstInt32 AxiomBridgedVstPlugin::canDo(char *text) {
    return true;
}

VstInt32 AxiomBridgedVstPlugin::getNumMidiInputChannels() {
    return 16;
}

bool AxiomBridgedVstPlugin::canParameterBeAutomated(VstInt32 index) {
    AxiomBackend::VstGuiMessage msg(AxiomBackend::VstGuiMessageType::GET_CAN_AUTOMATE_PARAMETER);
    msg.data.getCanAutomateParameter.index = index;
    channel.guiVstToApp.pushWhenAvailable(msg);

    while (true) {
        auto nextMessage = guiDispatcher.waitForNext();
        if (nextMessage.type == AxiomBackend::AppGuiMessageType::CAN_AUTOMATE_PARAMETER) {
            return nextMessage.data.canAutomateParameter.canAutomate;
        }
    }
}

float* AxiomBridgedVstPlugin::getInputBufferPtr(size_t inputIndex) {
    return reinterpret_cast<float *>(ioBuffer.data()) + inputIndex * AxiomBackend::IO_BUFFER_SIZE;
}

float* AxiomBridgedVstPlugin::getOutputBufferPtr(size_t outputIndex) {
    return reinterpret_cast<float *>(ioBuffer.data()) + AxiomBackend::IO_BUFFER_SIZE * (inputCount + outputIndex);
}

void AxiomBridgedVstPlugin::handleSetParameter(AxiomBackend::AppGuiSetParameterMessage message) {
    beginEdit(message.index);
    setParameterAutomated(message.index, message.value);
    endEdit(message.index);
}

void AxiomBridgedVstPlugin::handleUpdateIo(AxiomBackend::AppGuiUpdateIoMessage message) {
    inputCount = message.inputCount;
    outputCount = message.outputCount;

    ioBuffer.detach();
    ioBuffer.setKey(AxiomBackend::getBufferStringKey(message.newMemoryId));
    auto attachSuccess = ioBuffer.attach();
    assert(attachSuccess);

    setNumInputs(message.inputCount);
    setNumOutputs(message.outputCount);
    ioChanged();
    updateDisplay();

    expectedInputCount = std::min(expectedInputCount, inputCount);
    expectedOutputCount = std::min(expectedOutputCount, outputCount);
}

AxiomBackend::DispatcherHandlerResult AxiomBridgedVstPlugin::dispatchGuiMessage(AxiomBackend::AppGuiMessage message) {
    switch (message.type) {
        case AxiomBackend::AppGuiMessageType::SET_PARAMETER:
            handleSetParameter(message.data.setParameter);
            break;
        case AxiomBackend::AppGuiMessageType::UPDATE_IO:
            handleUpdateIo(message.data.updateIo);
            break;
        case AxiomBackend::AppGuiMessageType::PARAMETER_VALUE:
        case AxiomBackend::AppGuiMessageType::PARAMETER_LABEL:
        case AxiomBackend::AppGuiMessageType::PARAMETER_DISPLAY:
        case AxiomBackend::AppGuiMessageType::PARAMETER_NAME:
        case AxiomBackend::AppGuiMessageType::CAN_AUTOMATE_PARAMETER:
        case AxiomBackend::AppGuiMessageType::FINISHED_SERIALIZE:
        case AxiomBackend::AppGuiMessageType::FINISHED_DESERIALIZE:
            // Used as a response, no need to handle here
            break;
        default:
            unreachable;
    }

    return AxiomBackend::DispatcherHandlerResult::CONTINUE;
}

AxiomBackend::DispatcherHandlerResult AxiomBridgedVstPlugin::dispatchAudioMessage(AxiomBackend::AppAudioMessage message) {
    switch (message.type) {
        case AxiomBackend::AppAudioMessageType::GENERATE_DONE:
            // Used as a response, no need to handle here
            break;
        default:
            unreachable;
    }

    return AxiomBackend::DispatcherHandlerResult::CONTINUE;
}
