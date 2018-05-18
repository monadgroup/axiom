#include "ExtractControl.h"

#include "../../util.h"

using namespace AxiomModel;

static Control::ControlType typeFromWireType(ConnectionWire::WireType wireType) {
    switch (wireType) {
        case ConnectionWire::WireType::NUM:
            return Control::ControlType::NUM_EXTRACT;
        case ConnectionWire::WireType::MIDI:
            return Control::ControlType::MIDI_EXTRACT;
    }

    unreachable;
}

ExtractControl::ExtractControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                               QString name, ConnectionWire::WireType wireType, ActiveSlotFlags activeSlots,
                               ModelRoot *root)
    : Control(typeFromWireType(wireType), wireType, uuid, parentUuid, pos, size, selected, std::move(name), root),
      _activeSlots(activeSlots) {
}

std::unique_ptr<ExtractControl> ExtractControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                       QSize size, bool selected, QString name,
                                                       AxiomModel::ConnectionWire::WireType wireType,
                                                       AxiomModel::ExtractControl::ActiveSlotFlags activeSlots,
                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<ExtractControl>(uuid, parentUuid, pos, size, selected, std::move(name), wireType,
                                            activeSlots, root);
}

std::unique_ptr<ExtractControl> ExtractControl::deserialize(QDataStream &stream, const QUuid &uuid,
                                                            const QUuid &parentUuid, QPoint pos, QSize size,
                                                            bool selected, QString name,
                                                            AxiomModel::ConnectionWire::WireType wireType,
                                                            AxiomModel::ModelRoot *root) {
    ActiveSlotFlags activeSlots;
    stream >> activeSlots;
    return create(uuid, parentUuid, pos, size, selected, std::move(name), wireType, activeSlots, root);
}

void ExtractControl::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Control::serialize(stream, parent, withContext);
    stream << _activeSlots;
}

void ExtractControl::setActiveSlots(AxiomModel::ExtractControl::ActiveSlotFlags activeSlots) {
    if (activeSlots != _activeSlots) {
        _activeSlots = activeSlots;
        activeSlotsChanged.trigger(activeSlots);
    }
}
