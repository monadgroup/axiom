#include "ControlRef.h"

#include "Frontend.h"
#include "editor/util.h"

using namespace MaximCompiler;

ControlRef::ControlRef(void *handle) : handle(handle) {}

QString ControlRef::getName() const {
    auto cStr = MaximFrontend::maxim_control_get_name(get());
    auto resultStr = QString::fromUtf8(cStr);
    MaximFrontend::maxim_destroy_string(cStr);
    return resultStr;
}

ControlType ControlRef::getType() const {
    return (ControlType) MaximFrontend::maxim_control_get_type(get());
}

bool ControlRef::getIsWritten() const {
    return MaximFrontend::maxim_control_get_written(get());
}

bool ControlRef::getIsRead() const {
    return MaximFrontend::maxim_control_get_read(get());
}

ControlType MaximCompiler::fromModelType(AxiomModel::Control::ControlType modelType) {
    switch (modelType) {
        case AxiomModel::Control::ControlType::NUM_PORTAL:
        case AxiomModel::Control::ControlType::NUM_SCALAR:
            return ControlType::Audio;
        case AxiomModel::Control::ControlType::MIDI_PORTAL:
        case AxiomModel::Control::ControlType::MIDI_SCALAR:
            return ControlType::Midi;
        case AxiomModel::Control::ControlType::NUM_EXTRACT:
            return ControlType::AudioExtract;
        case AxiomModel::Control::ControlType::MIDI_EXTRACT:
            return ControlType::MidiExtract;
        case AxiomModel::Control::ControlType::SCOPE:
            return ControlType::Scope;
    }

    unreachable;
}

AxiomModel::Control::ControlType MaximCompiler::toModelType(MaximCompiler::ControlType type) {
    switch (type) {
        case ControlType::Audio:
            return AxiomModel::Control::ControlType::NUM_SCALAR;
        case ControlType::Midi:
            return AxiomModel::Control::ControlType::MIDI_SCALAR;
        case ControlType::Scope:
            return AxiomModel::Control::ControlType::SCOPE;
        case ControlType::AudioExtract:
            return AxiomModel::Control::ControlType::NUM_EXTRACT;
        case ControlType::MidiExtract:
            return AxiomModel::Control::ControlType::MIDI_EXTRACT;
        default:
            assert(false);
    }

    unreachable;
}
