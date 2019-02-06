#include "ValueSerializer.h"

using namespace AxiomModel;

void ValueSerializer::serializeNum(const AxiomModel::NumValue &val, QDataStream &stream) {
    stream << val.left;
    stream << val.right;
    stream << (uint8_t) val.form;
}

NumValue ValueSerializer::deserializeNum(QDataStream &stream, uint32_t version) {
    // versions < 3 stored an active flag
    if (version < 3) {
        bool dummy;
        stream >> dummy;
    }

    NumValue val;

    // versions < 6 stored left/right as floats instead of doubles
    if (version < 6) {
        float leftFloat;
        stream >> leftFloat;
        float rightFloat;
        stream >> rightFloat;

        val.left = leftFloat;
        val.right = rightFloat;
    } else {
        stream >> val.left;
        stream >> val.right;
    }

    uint8_t intForm;
    stream >> intForm;
    val.form = (FormType) intForm;
    return val;
}

void ValueSerializer::serializeMidiEvent(const AxiomModel::MidiEventValue &val, QDataStream &stream) {
    stream << (uint8_t) val.event;
    stream << val.channel;
    stream << val.note;
    stream << val.param;
}

MidiEventValue ValueSerializer::deserializeMidiEvent(QDataStream &stream, uint32_t version) {
    MidiEventValue val;
    uint8_t intEvent;
    stream >> intEvent;
    val.event = (MidiEventType) intEvent;
    stream >> val.channel;
    stream >> val.note;
    stream >> val.param;
    return val;
}

void ValueSerializer::serializeMidi(const AxiomModel::MidiValue &val, QDataStream &stream) {
    stream << val.count;
    for (uint8_t i = 0; i < val.count; i++) {
        serializeMidiEvent(val.events[i], stream);
    }
}

MidiValue ValueSerializer::deserializeMidi(QDataStream &stream, uint32_t version) {
    // versions < 3 stored an active flag
    if (version < 3) {
        bool dummy;
        stream >> dummy;
    }

    MidiValue val;
    stream >> val.count;
    for (uint8_t i = 0; i < val.count; i++) {
        val.events[i] = deserializeMidiEvent(stream, version);
    }
    return val;
}
