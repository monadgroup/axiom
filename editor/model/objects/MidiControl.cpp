#include "MidiControl.h"

using namespace AxiomModel;

MidiControl::MidiControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                         QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                         AxiomModel::ModelRoot *root)
    : Control(ControlType::MIDI_SCALAR, ConnectionWire::WireType::MIDI, uuid, parentUuid, pos, size, selected,
              std::move(name), showName, exposerUuid, exposingUuid, root) {}

std::unique_ptr<MidiControl> MidiControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                 bool selected, QString name, bool showName, const QUuid &exposerUuid,
                                                 const QUuid &exposingUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<MidiControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid,
                                         exposingUuid, root);
}
