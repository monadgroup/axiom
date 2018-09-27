#include "AxiomVstPlugin.h"

using namespace AxiomBackend;

AxiomApplication application;

extern "C" {
AEffect *VSTPluginMain(audioMasterCallback audioMaster) {
    if (!audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0)) return nullptr;
    auto effect = new AxiomVstPlugin(audioMaster);
    return effect->getAeffect();
}
}

AxiomVstPlugin::AxiomVstPlugin(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 255), backend(this), editor(&application, &backend) {
    isSynth();
    // setNumInputs(0);
    // setNumOutputs(2);
    setUniqueID(0x41584F4D); // 'AXOM'
    programsAreChunks();
    canProcessReplacing();

    setEditor(&editor);
}

AxiomVstPlugin::~AxiomVstPlugin() {
    // prevent the AudioEffect destructor trying to delete the editor
    setEditor(nullptr);
}

void AxiomVstPlugin::suspend() {
    saveBuffer.clear();
}

void AxiomVstPlugin::resume() {
    saveBuffer.clear();
}

void AxiomVstPlugin::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    auto timeInfo = getTimeInfo(kVstTempoValid);
    if (timeInfo->flags & kVstTempoValid) {
        backend.setBpm((float) timeInfo->tempo);
    }

    auto sampleFrames64 = (uint64_t) sampleFrames;
    uint64_t processPos = 0;
    while (processPos < sampleFrames64) {
        auto lock = backend.lockRuntime();
        auto sampleAmount = backend.beginGenerate();
        auto endProcessPos = processPos + sampleAmount;
        if (endProcessPos > sampleFrames64) endProcessPos = sampleFrames64;

        for (auto i = processPos; i < endProcessPos; i++) {
            for (size_t inputIndex = 0; inputIndex < backend.audioInputs.size(); inputIndex++) {
                const auto &input = backend.audioInputs[inputIndex];
                if (input) {
                    auto &inputNum = **input->value;
                    inputNum.left = inputs[inputIndex * 2][i];
                    inputNum.right = inputs[inputIndex * 2 + 1][i];
                    inputNum.form = AxiomBackend::NumForm::OSCILLATOR;
                }
            }

            backend.generate();

            for (size_t outputIndex = 0; outputIndex < backend.audioOutputs.size(); outputIndex++) {
                const auto &output = backend.audioOutputs[outputIndex];
                auto leftIndex = outputIndex * 2;
                auto rightIndex = leftIndex + 1;

                if (output) {
                    auto outputNum = **output->value;
                    outputs[leftIndex][i] = outputNum.left;
                    outputs[rightIndex][i] = outputNum.right;
                } else {
                    outputs[leftIndex][i] = 0;
                    outputs[rightIndex][i] = 0;
                }
            }

            if (backend.midiInputPortal != -1 && i == processPos) {
                backend.clearMidi((size_t) backend.midiInputPortal);
            }
        }

        processPos = endProcessPos;
    }
}

VstInt32 AxiomVstPlugin::processEvents(VstEvents *events) {
    if (backend.midiInputPortal == -1) {
        return 0;
    }

    for (auto i = 0; i < events->numEvents; i++) {
        auto event = events->events[i];
        if (event->type != kVstMidiType) continue;

        auto midiEvent = (VstMidiEvent *) event;
        auto midiStatus = midiEvent->midiData[0];
        auto midiData1 = midiEvent->midiData[1];
        auto midiData2 = midiEvent->midiData[2];

        auto eventType = (uint8_t)(midiStatus & 0xF0);
        auto eventChannel = (uint8_t)(midiStatus & 0x0F);

        MidiEvent remappedEvent;
        remappedEvent.channel = eventChannel;

        switch (eventType) {
        case 0x80: // note off
            remappedEvent.event = MidiEventType::NOTE_OFF;
            remappedEvent.note = (uint8_t) midiData1;
            backend.queueMidiEvent((size_t) event->deltaFrames, (size_t) backend.midiInputPortal, remappedEvent);
            break;
        case 0x90: // note on
            remappedEvent.event = MidiEventType::NOTE_ON;
            remappedEvent.note = (uint8_t) midiData1;
            remappedEvent.param = (uint8_t)(midiData2 * 2); // MIDI velocity is 0-127, we need 0-255
            backend.queueMidiEvent((size_t) event->deltaFrames, (size_t) backend.midiInputPortal, remappedEvent);
            break;
        case 0xA0: // polyphonic aftertouch
            remappedEvent.event = MidiEventType::POLYPHONIC_AFTERTOUCH;
            remappedEvent.note = (uint8_t) midiData1;
            remappedEvent.param = (uint8_t)(midiData2 * 2); // MIDI aftertouch pressure is 0-127, we need 0-255
            backend.queueMidiEvent((size_t) event->deltaFrames, (size_t) backend.midiInputPortal, remappedEvent);
            break;
        case 0xB0: // control mode change
            // all notes off
            if (midiData1 == 0x7B) {
                backend.clearNotes(0);
            }
            break;
        case 0xD0: // channel aftertouch
            remappedEvent.event = MidiEventType::CHANNEL_AFTERTOUCH;
            remappedEvent.param = (uint8_t)(midiData1 * 2); // MIDI aftertouch pressure is 0-127, we need 0-255
            backend.queueMidiEvent((size_t) event->deltaFrames, (size_t) backend.midiInputPortal, remappedEvent);
            break;
        case 0xE0: // pitch wheel
        {
            remappedEvent.event = MidiEventType::PITCH_WHEEL;

            // Pitch is 0-0x3FFF stored across the two bytes, we need 0-255
            auto pitch = ((uint16_t) midiData2 << 7) | (uint16_t) midiData1;
            remappedEvent.param = (uint8_t)(pitch / 16383.f * 255.f);
            backend.queueMidiEvent((size_t) event->deltaFrames, (size_t) backend.midiInputPortal, remappedEvent);
            break;
        }
        default:;
        }
    }
    return 0;
}

void AxiomVstPlugin::setSampleRate(float sampleRate) {
    backend.setSampleRate(sampleRate);
}

void AxiomVstPlugin::setParameter(VstInt32 index, float value) {
    if ((size_t) index >= backend.automationInputs.size()) return;
    auto &param = backend.automationInputs[index];
    if (param) {
        auto val = *param->value;
        val->left = value;
        val->right = value;
        val->form = NumForm::CONTROL;
    }
}

float AxiomVstPlugin::getParameter(VstInt32 index) {
    if ((size_t) index >= backend.automationInputs.size()) return 0;
    auto &param = backend.automationInputs[index];
    return param ? (*param->value)->left : 0;
}

void AxiomVstPlugin::getParameterLabel(VstInt32 index, char *label) {
    if ((size_t) index >= backend.automationInputs.size()) return;
    auto &param = backend.automationInputs[index];
    if (param) {
        auto val = *param->value;
        vst_strncpy(label, backend.formatNumForm(val->left, val->form), kVstMaxParamStrLen);
    }
}

void AxiomVstPlugin::getParameterDisplay(VstInt32 index, char *text) {
    if ((size_t) index >= backend.automationInputs.size()) return;
    auto &param = backend.automationInputs[index];
    if (param) {
        auto val = *param->value;
        vst_strncpy(text, backend.formatNum(*val, false).c_str(), kVstMaxParamStrLen);
    }
}

void AxiomVstPlugin::getParameterName(VstInt32 index, char *text) {
    if ((size_t) index >= backend.automationInputs.size()) return;
    auto &param = backend.automationInputs[index];
    if (param) {
        vst_strncpy(text, param->name.c_str(), kVstMaxParamStrLen);
    }
}

VstInt32 AxiomVstPlugin::getChunk(void **data, bool) {
    // the VST specification requires that the returned buffer is valid until the next suspend/resume call,
    // so we store the QByteArray and clear it above.
    saveBuffer = backend.serialize();
    *data = saveBuffer.data();
    return saveBuffer.size();
}

VstInt32 AxiomVstPlugin::setChunk(void *data, VstInt32 byteSize, bool) {
    auto byteArray = QByteArray::fromRawData((char *) data, byteSize);
    backend.deserialize(&byteArray);
    return 0;
}

bool AxiomVstPlugin::getEffectName(char *name) {
    vst_strncpy(name, PRODUCT_NAME, kVstMaxEffectNameLen);
    return true;
}

bool AxiomVstPlugin::getVendorString(char *text) {
    vst_strncpy(text, COMPANY_NAME, kVstMaxVendorStrLen);
    return true;
}

bool AxiomVstPlugin::getProductString(char *text) {
    vst_strncpy(text, FILE_DESCRIPTION, kVstMaxProductStrLen);
    return true;
}

VstInt32 AxiomVstPlugin::getVendorVersion() {
    return 1;
}

VstPlugCategory AxiomVstPlugin::getPlugCategory() {
    return kPlugCategSynth;
}

VstInt32 AxiomVstPlugin::canDo(char *) {
    return true;
}

VstInt32 AxiomVstPlugin::getNumMidiInputChannels() {
    return 16;
}

bool AxiomVstPlugin::canParameterBeAutomated(VstInt32 index) {
    return (size_t) index < backend.automationInputs.size();
}

void AxiomVstPlugin::backendSetParameter(size_t parameter, AxiomBackend::NumValue value) {
    beginEdit(parameter);
    setParameterAutomated(parameter, value.left);
    endEdit(parameter);
}

void AxiomVstPlugin::backendUpdateIo() {
    std::cout << backend.audioInputs.size() << " inputs" << std::endl;
    std::cout << backend.audioOutputs.size() << " outputs" << std::endl;

    setNumInputs(2 * backend.audioInputs.size());
    setNumOutputs(2 * backend.audioOutputs.size());
    ioChanged();
}
