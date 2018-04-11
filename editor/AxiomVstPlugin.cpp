#include "AxiomVstPlugin.h"

#include <QtCore/QBuffer>
#include <iostream>
#include <QtWidgets/QMessageBox>

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

void AxiomVstPlugin::suspend() {
    saveBuffer.reset();
}

void AxiomVstPlugin::resume() {
    saveBuffer.reset();
}

void AxiomVstPlugin::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    runtime.fillBuffer(outputs, (size_t) sampleFrames);
}

VstInt32 AxiomVstPlugin::processEvents(VstEvents *ev) {
    for (auto i = 0; i < ev->numEvents; i++) {
        auto evt = ev->events[i];
        if (evt->type != kVstMidiType) continue;

        auto midiEvent = (VstMidiEvent *) evt;
        auto midiStatus = midiEvent->midiData[0];
        auto midiData1 = midiEvent->midiData[1];
        auto midiData2 = midiEvent->midiData[2];

        auto eventType = (uint8_t) (midiStatus & 0xF0);
        auto eventChannel = (uint8_t) (midiStatus & 0x0F);

        MaximRuntime::MidiEventValue event{};
        event.channel = eventChannel;
        event.note = (uint8_t) midiData1;
        event.param = (uint8_t) midiData2;

        switch (eventType) {
            case 0x80: // note off
                event.event = MaximCommon::MidiEventType::NOTE_OFF;
                runtime.queueEvent({evt->deltaFrames, event});
                break;
            case 0x90: // note on
                event.event = MaximCommon::MidiEventType::NOTE_ON;
                runtime.queueEvent({evt->deltaFrames, event});
                break;
            case 0xA0: // polyphonic aftertouch
                event.event = MaximCommon::MidiEventType::POLYPHONIC_AFTERTOUCH;
                runtime.queueEvent({evt->deltaFrames, event});
                break;
            case 0xB0: // control mode change

                // all notes off
                if (midiData1 == 0x7B) {
                    runtime.clearEvents();
                }

                break;
            case 0xD0: // channel aftertouch
                event.event = MaximCommon::MidiEventType::CHANNEL_AFTERTOUCH;
                event.param = (uint8_t) midiData1;
                runtime.queueEvent({evt->deltaFrames, event});
                break;
            case 0xE0: // pitch wheel
                event.event = MaximCommon::MidiEventType::PITCH_WHEEL;
                runtime.queueEvent({evt->deltaFrames, event});
                break;
            default:;
        }
    }
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
    saveBuffer = std::make_unique<QByteArray>();
    QBuffer buffer(saveBuffer.get());
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);

    std::cout << "Serializing chunk" << std::endl;
    project.serialize(stream);
    std::cout << "Finished serializing, with " << saveBuffer->size() << " bytes" << std::endl;

    buffer.close();

    *data = saveBuffer->data();
    return saveBuffer->size();
}

VstInt32 AxiomVstPlugin::setChunk(void *data, VstInt32 byteSize, bool isPreset) {
    QByteArray byteArray = QByteArray::fromRawData((char *) data, byteSize);
    QDataStream stream(&byteArray, QIODevice::ReadOnly);

    try {
        project.load(stream);
    } catch (AxiomModel::DeserializeInvalidFileException) {
        QMessageBox(QMessageBox::Critical, "Failed to load data",
                    "Your DAW just gave Axiom an invalid project file (bad magic header).\n"
                    "Maybe it's corrupt?", QMessageBox::Ok).exec();
    } catch (AxiomModel::DeserializeInvalidSchemaException) {
        QMessageBox(QMessageBox::Critical, "Failed to load data",
                    "Your DAW just gave Axiom a project file saved with an outdated project format.\n\n"
                    "Unfortunately Axiom currently doesn't support loading old project formats.\n"
                    "If you really want this feature, maybe make a pull request (https://github.com/monadgroup/axiom/pulls)"
                    " or report an issue (https://github.com/monadgroup/axiom/issues).", QMessageBox::Ok).exec();
    }

    return 0;
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
