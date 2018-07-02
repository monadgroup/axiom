#include "Value.h"

using namespace AxiomModel;

QDataStream &AxiomModel::operator<<(QDataStream &stream, const NumValue &val) {
    // todo: backwards compatability: stream << val.active;
    stream << val.left;
    stream << val.right;
    stream << (uint8_t) val.form;
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, NumValue &val) {
    stream >> val.left;
    stream >> val.right;

    uint8_t intForm;
    stream >> intForm;
    val.form = (FormType) intForm;
    return stream;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const MidiEventValue &val) {
    stream << (uint8_t) val.event;
    stream << val.channel;
    stream << val.note;
    stream << val.param;
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, MidiEventValue &val) {
    uint8_t intEvent;
    stream >> intEvent;
    val.event = (MidiEventType) intEvent;

    stream >> val.channel;
    stream >> val.note;
    stream >> val.param;
    return stream;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const MidiValue &val) {
    // todo: backwards compatability: stream << val.active;
    stream << val.count;

    for (uint8_t i = 0; i < val.count; i++) {
        stream << val.events[i];
    }
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, MidiValue &val) {
    stream >> val.count;

    for (uint8_t i = 0; i < val.count; i++) {
        stream >> val.events[i];
    }
    return stream;
}
