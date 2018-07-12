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
                             AxiomModel::PortalControl::PortalType portalType, AxiomModel::ModelRoot *root)
    : Control(typeFromWireType(wireType), wireType, uuid, parentUuid, pos, size, selected, std::move(name), showName,
              exposerUuid, exposingUuid, root),
      _portalType(portalType) {
}

std::unique_ptr<PortalControl> PortalControl::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                                     bool selected, QString name, bool showName,
                                                     const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                     AxiomModel::ConnectionWire::WireType wireType,
                                                     AxiomModel::PortalControl::PortalType portalType,
                                                     AxiomModel::ModelRoot *root) {
    return std::make_unique<PortalControl>(uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                           exposerUuid, exposingUuid, wireType, portalType, root);
}

std::unique_ptr<PortalControl> PortalControl::deserialize(QDataStream &stream, const QUuid &uuid,
                                                          const QUuid &parentUuid, QPoint pos, QSize size,
                                                          bool selected, QString name, bool showName,
                                                          const QUuid &exposerUuid, const QUuid &exposingUuid,
                                                          AxiomModel::ConnectionWire::WireType wireType,
                                                          ReferenceMapper *ref, AxiomModel::ModelRoot *root) {
    uint8_t portalTypeInt;
    stream >> portalTypeInt;
    return create(uuid, parentUuid, pos, size, selected, std::move(name), showName, exposerUuid, exposingUuid, wireType,
                  (PortalType) portalTypeInt, root);
}

void PortalControl::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    Control::serialize(stream, parent, withContext);
    stream << (uint8_t) _portalType;
}
