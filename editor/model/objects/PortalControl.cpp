#include "PortalControl.h"
#include "../../util.h"

using namespace AxiomModel;

static Control::ControlType typeFromWireType(ConnectionWire::WireType wireType) {
    switch (wireType) {
    case ConnectionWire::WireType::NUM:
        return Control::ControlType::NUM_PORTAL;
    case ConnectionWire::WireType::MIDI:
        return Control::ControlType::MIDI_PORTAL;
    }
    unreachable;
}

PortalControl::PortalControl(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                             QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                             AxiomModel::ConnectionWire::WireType wireType,
                             AxiomModel::PortalControl::PortalType portalType, uint64_t portalId,
                             AxiomModel::ModelRoot *root)
    : Control(typeFromWireType(wireType), wireType, QSize(1, 1), uuid, parentUuid, pos, size, selected, std::move(name),
              showName, exposerUuid, exposingUuid, root),
      _portalType(portalType), _portalId(portalId) {}

std::unique_ptr<PortalControl> PortalControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                     bool selected, QString name, bool showName,
                                                     const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                     AxiomModel::ConnectionWire::WireType wireType,
                                                     AxiomModel::PortalControl::PortalType portalType,
                                                     uint64_t portalId, AxiomModel::ModelRoot *root) {
    return std::make_unique<PortalControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                           exposerUuid, exposingUuid, wireType, portalType, portalId, root);
}
