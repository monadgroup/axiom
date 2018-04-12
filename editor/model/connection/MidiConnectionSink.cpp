#include "MidiConnectionSink.h"

using namespace AxiomModel;

MidiConnectionSink::MidiConnectionSink(NodeControl *control) : ConnectionSink(Type::MIDI, control) {

}

void MidiConnectionSink::setValue(MaximRuntime::MidiValue value) {
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}
