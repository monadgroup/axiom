#include <iostream>
#include "NodeMidiControl.h"

#include "compiler/runtime/Control.h"
#include "compiler/runtime/ControlGroup.h"
#include "compiler/runtime/Node.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

NodeMidiControl::NodeMidiControl(Node *node, QString name, QPoint pos, QSize size)
    : NodeControl(node, std::move(name), pos, size), m_sink(this) {
    initSink();

    connect(&m_sink, &MidiConnectionSink::valueChanged,
            this, &NodeMidiControl::valueChanged);
}

void NodeMidiControl::doRuntimeUpdate() {
    saveValue();
}

void NodeMidiControl::setValue(MaximRuntime::MidiValue value, bool setRuntime) {
    m_sink.setValue(value);
    if (setRuntime) restoreValue();
}

void NodeMidiControl::saveValue() {
    if (!runtime() || !runtime()->group()) return;
    setValue(runtime()->group()->getMidiValue());
}

void NodeMidiControl::restoreValue() {
    if (!runtime() || !runtime()->group()) return;
    runtime()->group()->setMidiValue(m_sink.value());
}

void NodeMidiControl::pushEvent(MaximRuntime::MidiEventValue event) {
    runtime()->group()->pushMidiEvent(event);
}

void NodeMidiControl::setMode(Mode mode) {
    if (mode != m_mode) {
        m_mode = mode;
        emit modeChanged(mode);
    }
}

void NodeMidiControl::serialize(QDataStream &stream) const {
    stream << m_sink.value();
}

void NodeMidiControl::deserialize(QDataStream &stream) {
    MaximRuntime::MidiValue val; stream >> val;
    m_sink.setValue(val);
}
