#include "ExtractControl.h"

#include "../../util.h"
#include "../Value.h"

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
                               QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                               ConnectionWire::WireType wireType, ActiveSlotFlags activeSlots, ModelRoot *root)
    : Control(typeFromWireType(wireType), wireType, QSize(1, 1), uuid, parentUuid, pos, size, selected, std::move(name),
              showName, exposerUuid, exposingUuid, root),
      _activeSlots(activeSlots) {}

std::unique_ptr<ExtractControl> ExtractControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                       QSize size, bool selected, QString name, bool showName,
                                                       const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                       AxiomModel::ConnectionWire::WireType wireType,
                                                       AxiomModel::ExtractControl::ActiveSlotFlags activeSlots,
                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<ExtractControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                            exposerUuid, exposingUuid, wireType, activeSlots, root);
}

QString ExtractControl::debugName() {
    return "ExtractControl '" + name() + "'";
}

void ExtractControl::setActiveSlots(AxiomModel::ExtractControl::ActiveSlotFlags activeSlots) {
    if (activeSlots != _activeSlots) {
        _activeSlots = activeSlots;
        activeSlotsChanged(activeSlots);
    }
}

void ExtractControl::doRuntimeUpdate() {
    if (!runtimePointers()) return;
    auto arr = (ArrayValue *) runtimePointers()->value;
    setActiveSlots(arr->flags);
}
