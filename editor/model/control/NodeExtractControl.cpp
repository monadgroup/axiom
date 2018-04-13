#include "NodeExtractControl.h"

#include "compiler/runtime/Control.h"
#include "compiler/runtime/ControlGroup.h"
#include "../../util.h"

using namespace AxiomModel;

NodeExtractControl::NodeExtractControl(Node *node, ConnectionSink::Type baseType,
                                       QString name, QPoint pos, QSize size)
    : NodeControl(node, std::move(name), pos, size), m_sink(this, baseType) {
    initSink();

    connect(&m_sink, &ExtractConnectionSink::activeSlotsChanged,
            this, &NodeExtractControl::activeSlotsChanged);
}

MaximCommon::ControlType NodeExtractControl::type() const {
    switch (m_sink.type) {
        case ConnectionSink::Type::NUMBER: return MaximCommon::ControlType::NUM_EXTRACT;
        case ConnectionSink::Type::MIDI: return MaximCommon::ControlType::MIDI_EXTRACT;
    }
    unreachable;
}

void NodeExtractControl::doRuntimeUpdate() {
    if (runtime()) {
        m_sink.setActiveSlots(runtime()->group()->getActiveFlags());
    }
}
