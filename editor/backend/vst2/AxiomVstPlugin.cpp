#include "AxiomVstPlugin.h"

#include <QtCore/QBuffer>
#include <QtWidgets/QMessageBox>

#include "editor/model/Value.h"
#include "resources/resource.h"
#include "widgets/surface/NodeSurfacePanel.h"

AudioEffect *createEffectInstance(audioMasterCallback audioMaster) {
    return new AxiomVstPlugin(audioMaster);
}

AxiomVstPlugin::AxiomVstPlugin(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 255), _editor(std::make_unique<AxiomModel::Project>()) {
    isSynth();
    setNumInputs(0);
    setNumOutputs(2);
    setUniqueID(0x41584F4D); // 'AXOM'
    programsAreChunks();
    canProcessReplacing();

    setEditor(&_editor);
}

AxiomVstPlugin::~AxiomVstPlugin() {
    // prevent the AudioEffect destructor trying to delete the editor
    setEditor(nullptr);
}

void AxiomVstPlugin::open() {}

void AxiomVstPlugin::suspend() {
    saveBuffer.reset();
}

void AxiomVstPlugin::resume() {
    saveBuffer.reset();
}

void AxiomVstPlugin::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) {
    /*auto timeInfo = getTimeInfo(kVstTempoValid);
    if (timeInfo->flags & kVstTempoValid) {
        runtime.setBpm(timeInfo->tempo);
    }

    runtime.fillBuffer(outputs, (size_t) sampleFrames);*/
}

VstInt32 AxiomVstPlugin::processEvents(VstEvents *ev) {
    /*for (auto i = 0; i < ev->numEvents; i++) {
        auto evt = ev->events[i];
        if (evt->type != kVstMidiType) continue;

        auto midiEvent = (VstMidiEvent *) evt;
        auto midiStatus = midiEvent->midiData[0];
        auto midiData1 = midiEvent->midiData[1];
        auto midiData2 = midiEvent->midiData[2];

        auto eventType = (uint8_t) (midiStatus & 0xF0);
        auto eventChannel = (uint8_t) (midiStatus & 0x0F);

        AxiomModel::MidiEventValue event{};
        event.channel = eventChannel;
        event.note = (uint8_t) midiData1;
        event.param = (uint8_t) midiData2;

        switch (eventType) {
            case 0x80: // note off
                event.event = AxiomModel::MidiEventType::NOTE_OFF;
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
    }*/

    return 0;
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
    /*auto node = runtime.mainSurface()->getAutomationNode(index);
    if (node == nullptr || node->control()->group() == nullptr) return;
    node->control()->group()->setNumValue({ true, value, value, MaximCommon::FormType::CONTROL });*/
}

float AxiomVstPlugin::getParameter(VstInt32 index) {
    /*auto node = runtime.mainSurface()->getAutomationNode(index);
    if (node == nullptr || node->control()->group() == nullptr) return 0;
    return node->control()->group()->getNumValue().left;*/
    return 0;
}

void AxiomVstPlugin::getParameterLabel(VstInt32 index, char *label) {
    vst_strncpy(label, "", kVstMaxParamStrLen);
}

void AxiomVstPlugin::getParameterDisplay(VstInt32 index, char *text) {
    float2string(getParameter(index), text, kVstMaxParamStrLen);
}

void AxiomVstPlugin::getParameterName(VstInt32 index, char *text) {
    /*auto node = runtime.mainSurface()->getAutomationNode(index);
    if (node == nullptr) vst_strncpy(text, "", kVstMaxParamStrLen);
    else vst_strncpy(text, node->name().c_str(), kVstMaxParamStrLen);*/
}

VstInt32 AxiomVstPlugin::getChunk(void **data, bool isPreset) {
    saveBuffer = std::make_unique<QByteArray>();
    QBuffer buffer(saveBuffer.get());
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);

    std::cout << "Serializing chunk" << std::endl;
    _editor.project()->serialize(stream);
    std::cout << "Finished serializing, with " << saveBuffer->size() << " bytes" << std::endl;

    buffer.close();

    *data = saveBuffer->data();
    return saveBuffer->size();
}

VstInt32 AxiomVstPlugin::setChunk(void *data, VstInt32 byteSize, bool isPreset) {
    QByteArray byteArray = QByteArray::fromRawData((char *) data, byteSize);
    QDataStream stream(&byteArray, QIODevice::ReadOnly);
    uint32_t readVersion = 0;
    auto newProject = AxiomModel::Project::deserialize(stream, &readVersion);

    if (!newProject) {
        if (readVersion) {
            QMessageBox(QMessageBox::Critical, "Failed to load project",
                        "This project was created with an incompatible version of Axiom.\n\n"
                        "Expected version: between " +
                            QString::number(AxiomModel::Project::minSchemaVersion) + " and " +
                            QString::number(AxiomModel::Project::schemaVersion) +
                            ", actual version: " + QString::number(readVersion) + ".",
                        QMessageBox::Ok)
                .exec();
        } else {
            QMessageBox(QMessageBox::Critical, "Failed to load project",
                        "This project is an invalid project file (bad magic header).\n"
                        "Maybe it's corrupt?",
                        QMessageBox::Ok)
                .exec();
        }
    } else {
        _editor.setProject(std::move(newProject));
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
    return true;
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

bool AxiomVstPlugin::canParameterBeAutomated(VstInt32 index) {
    // auto node = runtime.mainSurface()->getAutomationNode(index);
    // return node != nullptr;
    return false;
}

void AxiomVstPlugin::fiddleParam(VstInt32 param) {
    beginEdit(param);
    setParameterAutomated(param, getParameter(param));
    endEdit(param);
}
