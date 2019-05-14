#include "AxiomVstPlugin.h"

#include <iostream>

#include "editor/backend/EventConverter.h"

using namespace AxiomBackend;

AxiomCommon::LazyInitializer<AxiomApplication> application;

extern "C" {
AEffect *VSTPluginMain(audioMasterCallback audioMaster) {
    if (!audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0)) return nullptr;
    auto effect = new AxiomVstPlugin(audioMaster);
    return effect->getAeffect();
}
}

bool isCompilingSynth() {
#ifdef AXIOM_VST2_IS_SYNTH
    return true;
#else
    return false;
#endif
}

AxiomVstPlugin::AxiomVstPlugin(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 255), appRef(application.get()), _backend(*this, isCompilingSynth()),
      editor(&*appRef, this) {
#ifdef AXIOM_VST2_IS_SYNTH
    isSynth();
    setUniqueID(0x41584F53);
#else
    setUniqueID(0x41584F45);
#endif

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
        _backend.setBpm((float) timeInfo->tempo);
    }

    auto sampleFrames64 = (uint64_t) sampleFrames;
    uint64_t processPos = 0;
    while (processPos < sampleFrames64) {
        auto context = _backend.beginGenerate();
        auto endProcessPos = processPos + context.maxGenerateCount();
        if (endProcessPos > sampleFrames64) endProcessPos = sampleFrames64;

        for (auto i = processPos; i < endProcessPos; i++) {
            for (size_t inputIndex = 0; inputIndex < expectedInputCount; inputIndex++) {
                const auto &input = _backend.audioInputs[inputIndex];
                if (input) {
                    auto &inputNum = *_backend.getAudioPortal(input->portalIndex);
                    inputNum.left = inputs[inputIndex * 2][i];
                    inputNum.right = inputs[inputIndex * 2 + 1][i];
                    inputNum.form = AxiomBackend::NumForm::OSCILLATOR;
                }
            }

            context.generate();

            for (size_t outputIndex = 0; outputIndex < expectedOutputCount; outputIndex++) {
                const auto &output = _backend.audioOutputs[outputIndex];
                auto leftIndex = outputIndex * 2;
                auto rightIndex = leftIndex + 1;

                if (output) {
                    auto outputNum = *_backend.getAudioPortal(output->portalIndex);
                    outputs[leftIndex][i] = (float) outputNum.left;
                    outputs[rightIndex][i] = (float) outputNum.right;
                } else {
                    outputs[leftIndex][i] = 0;
                    outputs[rightIndex][i] = 0;
                }
            }

            if (_backend.midiInputPortal != -1 && i == processPos) {
                _backend.clearMidi((size_t) _backend.midiInputPortal);
            }
        }

        processPos = endProcessPos;
    }

    expectedInputCount = _backend.audioInputs.size();
    expectedOutputCount = _backend.audioOutputs.size();
}

VstInt32 AxiomVstPlugin::processEvents(VstEvents *events) {
    if (_backend.midiInputPortal == -1) {
        return 0;
    }

    for (auto i = 0; i < events->numEvents; i++) {
        auto event = events->events[i];
        if (event->type != kVstMidiType) continue;

        auto midiEvent = *reinterpret_cast<int32_t *>(((VstMidiEvent *) event)->midiData);
        if (auto remappedEvent = convertFromMidi(midiEvent)) {
            _backend.queueMidiEvent((size_t) event->deltaFrames, (size_t) _backend.midiInputPortal, *remappedEvent);
        }
    }
    return 0;
}

void AxiomVstPlugin::setSampleRate(float sampleRate) {
    _backend.setSampleRate(sampleRate);
}

void AxiomVstPlugin::setParameter(VstInt32 index, float value) {
    if ((size_t) index >= _backend.automationInputs.size()) return;
    auto &param = _backend.automationInputs[index];
    if (param) {
        auto val = _backend.getAudioPortal(param->portalIndex);
        val->left = value;
        val->right = value;
        val->form = NumForm::CONTROL;
    }
}

float AxiomVstPlugin::getParameter(VstInt32 index) {
    if ((size_t) index >= _backend.automationInputs.size()) return 0;
    auto &param = _backend.automationInputs[index];
    return param ? _backend.getAudioPortal(param->portalIndex)->left : 0;
}

void AxiomVstPlugin::getParameterLabel(VstInt32 index, char *label) {
    if ((size_t) index >= _backend.automationInputs.size()) return;
    auto &param = _backend.automationInputs[index];
    if (param) {
        auto val = _backend.getAudioPortal(param->portalIndex);
        vst_strncpy(label, _backend.formatNumForm(val->left, val->form), kVstMaxParamStrLen);
    }
}

void AxiomVstPlugin::getParameterDisplay(VstInt32 index, char *text) {
    if ((size_t) index >= _backend.automationInputs.size()) return;
    auto &param = _backend.automationInputs[index];
    if (param) {
        auto val = _backend.getAudioPortal(param->portalIndex);
        vst_strncpy(text, _backend.formatNum(*val, false).c_str(), kVstMaxParamStrLen);
    }
}

void AxiomVstPlugin::getParameterName(VstInt32 index, char *text) {
    if ((size_t) index >= _backend.automationInputs.size()) return;
    auto &param = _backend.automationInputs[index];
    if (param) {
        vst_strncpy(text, param->name.c_str(), kVstMaxParamStrLen);
    }
}

VstInt32 AxiomVstPlugin::getChunk(void **data, bool) {
    // the VST specification requires that the returned buffer is valid until the next suspend/resume call,
    // so we store the QByteArray and clear it above.
    saveBuffer = _backend.serialize([this](QDataStream &stream) {
        _backend.audioInputs.serialize(stream);
        _backend.audioOutputs.serialize(stream);
        _backend.automationInputs.serialize(stream);
    });
    *data = saveBuffer.data();
    return saveBuffer.size();
}

VstInt32 AxiomVstPlugin::setChunk(void *data, VstInt32 byteSize, bool) {
    auto byteArray = QByteArray::fromRawData((char *) data, byteSize);
    std::cout << "Starting deserialize" << std::endl;
    _backend.deserialize(&byteArray, [this](QDataStream &stream, uint32_t version) {
        _backend.audioInputs = AxiomBackend::NumParameters::deserialize(stream, version);
        _backend.audioOutputs = AxiomBackend::NumParameters::deserialize(stream, version);
        _backend.automationInputs = AxiomBackend::NumParameters::deserialize(stream, version);
    });
    std::cout << "Finished deserialize" << std::endl;
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
    // A canDo attitude
    return true;
}

VstInt32 AxiomVstPlugin::getNumMidiInputChannels() {
    return 16;
}

bool AxiomVstPlugin::canParameterBeAutomated(VstInt32 index) {
    return (size_t) index < _backend.automationInputs.size();
}

void AxiomVstPlugin::adapterSetParameter(size_t parameter, AxiomBackend::NumValue value) {
    beginEdit(parameter);
    setParameterAutomated(parameter, value.left);
    endEdit(parameter);
}

void AxiomVstPlugin::adapterUpdateIo() {
    std::cout << "Got adapterUpdateIo" << std::endl;
    setNumInputs(2 * _backend.audioInputs.size());
    setNumOutputs(2 * _backend.audioOutputs.size());
    ioChanged();
    updateDisplay();

    expectedInputCount = std::min(expectedInputCount, _backend.audioInputs.size());
    expectedOutputCount = std::min(expectedOutputCount, _backend.audioOutputs.size());
    std::cout << "Finished adapterUpdateIo" << std::endl;
}
