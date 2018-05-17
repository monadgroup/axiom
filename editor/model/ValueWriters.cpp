#include "ValueWriters.h"

using namespace AxiomModel;

QDataStream &AxiomModel::operator<<(QDataStream &stream, const MaximRuntime::NumValue &val) {
    stream << val.active;
    stream << val.left;
    stream << val.right;
    stream << (uint8_t) val.form;
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, MaximRuntime::NumValue &val) {
    stream >> val.active;
    stream >> val.left;
    stream >> val.right;

    uint8_t intForm;
    stream >> intForm;
    val.form = (MaximCommon::FormType) intForm;
    return stream;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const MaximRuntime::MidiEventValue &val) {
    stream << (uint8_t) val.event;
    stream << val.channel;
    stream << val.note;
    stream << val.param;
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, MaximRuntime::MidiEventValue &val) {
    uint8_t intEvent;
    stream >> intEvent;
    val.event = (MaximCommon::MidiEventType) intEvent;

    stream >> val.channel;
    stream >> val.note;
    stream >> val.param;
    return stream;
}

QDataStream &AxiomModel::operator<<(QDataStream &stream, const MaximRuntime::MidiValue &val) {
    stream << val.active;
    stream << val.count;

    for (uint8_t i = 0; i < val.count; i++) {
        stream << val.events[i];
    }
    return stream;
}

QDataStream &AxiomModel::operator>>(QDataStream &stream, MaximRuntime::MidiValue &val) {
    stream >> val.active;
    stream >> val.count;

    for (uint8_t i = 0; i < val.count; i++) {
        stream >> val.events[i];
    }
    return stream;
}
