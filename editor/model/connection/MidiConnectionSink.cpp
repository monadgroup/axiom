#include "MidiConnectionSink.h"

using namespace AxiomModel;

MidiConnectionSink::MidiConnectionSink(MaximRuntime::Control *runtime) : ConnectionSink(Type::MIDI, runtime) {

}

void MidiConnectionSink::setValue(MaximRuntime::MidiValue value) {
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}
