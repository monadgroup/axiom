#include "AxiomVstPlugin.h"

#include "resources/resource.h"
#include "AxiomVstEditor.h"
#include "compiler/runtime/GeneratableModuleClass.h"
#include "compiler/runtime/ControlGroup.h"
#include "compiler/codegen/Operator.h"
#include "compiler/codegen/Converter.h"
#include "compiler/codegen/Control.h"
#include "compiler/codegen/Function.h"

AudioEffect *createEffectInstance(audioMasterCallback audioMaster) {
    return new AxiomVstPlugin(audioMaster);
}

AxiomVstPlugin::AxiomVstPlugin(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 0), project(&runtime) {
    isSynth();
    programsAreChunks();
    canProcessReplacing();
    setNumInputs(0);
    setNumOutputs(2);
    setUniqueID(0x41584F4D); // 'AXOM'
    setEditor(new AxiomVstEditor(this));
}

void AxiomVstPlugin::open() {

}

void AxiomVstPlugin::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    // while there are events to be distributed,
    // fill buffer up until next event pos, then stuff events into input

    runtime.fillBuffer(outputs, (size_t) sampleFrames);
}

VstInt32 AxiomVstPlugin::processEvents(VstEvents *ev) {
    // todo
}

void AxiomVstPlugin::setProgramName(char *name) {
    // todo
}

void AxiomVstPlugin::getProgramName(char *name) {
    // todo
}

void AxiomVstPlugin::setSampleRate(float sampleRate) {
    // todo
}

void AxiomVstPlugin::setParameter(VstInt32 index, float value) {
    // todo
}

float AxiomVstPlugin::getParameter(VstInt32 index) {
    // todo
}

void AxiomVstPlugin::getParameterLabel(VstInt32 index, char *label) {
    // todo
}

void AxiomVstPlugin::getParameterDisplay(VstInt32 index, char *text) {
    // todo
}

void AxiomVstPlugin::getParameterName(VstInt32 index, char *text) {
    // todo
}

VstInt32 AxiomVstPlugin::getChunk(void **data, bool isPreset) {
    // todo
}

VstInt32 AxiomVstPlugin::setChunk(void *data, VstInt32 byteSize, bool isPreset) {
    // todo
}

bool AxiomVstPlugin::getEffectName(char *name) {
    vst_strncpy(name, VER_INTERNALNAME_STR, kVstMaxEffectNameLen);
    return true;
}

bool AxiomVstPlugin::getVendorString(char *text) {
    vst_strncpy(text, "MONAD Demogroup", kVstMaxVendorStrLen);
    return true;
}

bool AxiomVstPlugin::getProductString(char *text) {
    vst_strncpy(text, VER_FILEDESCRIPTION_STR, kVstMaxProductStrLen);
}

VstInt32 AxiomVstPlugin::getVendorVersion() {
    return 1;
}

VstPlugCategory AxiomVstPlugin::getPlugCategory() {
    return kPlugCategSynth;
}

VstInt32 AxiomVstPlugin::canDo(char *text) {
    return text == "sendVstEvents" || text == "sendVstMidiEvent" || text == "receiveVstEvents" ||
           text == "receiveVstMidiEvent";
}

VstInt32 AxiomVstPlugin::getNumMidiInputChannels() {
    return 16;
}
