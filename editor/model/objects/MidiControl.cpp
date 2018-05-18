#include "MidiControl.h"

using namespace AxiomModel;

MidiControl::MidiControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                         QString name, AxiomModel::ModelRoot *root)
    : Control(ControlType::MIDI_SCALAR, ConnectionWire::WireType::MIDI, uuid, parentUuid, pos, size, selected,
              std::move(name), root) {
}

std::unique_ptr<MidiControl> MidiControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                 bool selected, QString name, AxiomModel::ModelRoot *root) {
    return std::make_unique<MidiControl>(uuid, parentUuid, pos, size, selected, std::move(name), root);
}

std::unique_ptr<MidiControl> MidiControl::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                      QPoint pos, QSize size, bool selected, QString name,
                                                      AxiomModel::ModelRoot *root) {
    return create(uuid, parentUuid, pos, size, selected, std::move(name), root);
}

void MidiControl::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Control::serialize(stream, parent, withContext);
}

void MidiControl::setValue(const MaximRuntime::MidiValue &value) {
    if (value != _value) {
        _value = value;
        valueChanged.trigger(value);
    }
}
