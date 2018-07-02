#include "Control.h"

#include "ControlSurface.h"
#include "Connection.h"
#include "GroupNode.h"
#include "NumControl.h"
#include "MidiControl.h"
#include "ExtractControl.h"
#include "PortalControl.h"
#include "ScopeControl.h"
#include "GroupSurface.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../ReferenceMapper.h"

using namespace AxiomModel;

Control::Control(AxiomModel::Control::ControlType controlType, AxiomModel::ConnectionWire::WireType wireType,
                 QUuid uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                 bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, AxiomModel::ModelRoot *root)
    : GridItem(&find(root->controlSurfaces(), parentUuid)->grid(), pos, size, selected),
      ModelObject(ModelType::CONTROL, uuid, parentUuid, root), _surface(find(root->controlSurfaces(), parentUuid)),
      _controlType(controlType), _wireType(wireType), _name(std::move(name)), _showName(showName),
      _exposerUuid(exposerUuid), _exposingUuid(exposingUuid),
      _connections(filterWatch(root->connections(), std::function([uuid](Connection *const &connection) {
          return connection->controlAUuid() == uuid || connection->controlBUuid() == uuid;
      }))), _connectedControls(
        mapFilterWatch(_connections, std::function([uuid](Connection *const &connection) -> std::optional<QUuid> {
            if (connection->controlAUuid() == uuid) return connection->controlBUuid();
            if (connection->controlBUuid() == uuid) return connection->controlAUuid();
            return std::optional<QUuid>();
        }))) {
    posChanged.connect(this, &Control::updateSinkPos);
    removed.connect(this, &Control::updateExposerRemoved);
    _surface->node()->posChanged.connect(this, &Control::updateSinkPos);

    if (!_exposingUuid.isNull()) {
        findLater<Control*>(root->controls(), _exposingUuid).then([uuid](Control *exposing) {
            exposing->setExposerUuid(uuid);
        });
    }
}

std::unique_ptr<Control> Control::createDefault(AxiomModel::Control::ControlType type, const QUuid &uuid,
                                                const QUuid &parentUuid, const QString &name,
                                                const QUuid &exposingUuid, AxiomModel::ModelRoot *root) {
    switch (type) {
        case Control::ControlType::NUM_SCALAR:
            return NumControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, NumControl::DisplayMode::KNOB, NumControl::Channel::BOTH, {
                0, 0, FormType::CONTROL
            }, root);
        case Control::ControlType::MIDI_SCALAR:
            return MidiControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, root);
        case Control::ControlType::NUM_EXTRACT:
            return ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, ConnectionWire::WireType::NUM, 0, root);
        case Control::ControlType::MIDI_EXTRACT:
            return ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid, ConnectionWire::WireType::MIDI, 0, root);
        case Control::ControlType::SCOPE:
            return ScopeControl::create(uuid, parentUuid, QPoint(0, 0), QSize(6, 6), false, name, true, QUuid(), exposingUuid, root);
        default: unreachable;
    }
}

std::unique_ptr<Control> Control::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                              ReferenceMapper *ref, AxiomModel::ModelRoot *root) {
    uint8_t controlTypeInt;
    stream >> controlTypeInt;

    QPoint pos;
    QSize size;
    bool selected;
    GridItem::deserialize(stream, pos, size, selected);

    QString name;
    stream >> name;

    bool showName;
    stream >> showName;

    QUuid exposerUuid;
    stream >> exposerUuid;
    exposerUuid = ref->mapUuid(exposerUuid);

    QUuid exposingUuid;
    stream >> exposingUuid;
    exposingUuid = ref->mapUuid(exposingUuid);

    switch ((ControlType) controlTypeInt) {
        case ControlType::NUM_SCALAR:
            return NumControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                exposerUuid, exposingUuid, ref, root);
        case ControlType::MIDI_SCALAR:
            return MidiControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                exposerUuid, exposingUuid, ref, root);
        case ControlType::NUM_EXTRACT:
            return ExtractControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name),
                                               showName, exposerUuid, exposingUuid, ConnectionWire::WireType::NUM, ref,
                                               root);
        case ControlType::MIDI_EXTRACT:
            return ExtractControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name),
                                               showName, exposerUuid, exposingUuid, ConnectionWire::WireType::MIDI, ref,
                                               root);
        case ControlType::NUM_PORTAL:
            return PortalControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                              exposerUuid, exposingUuid, ConnectionWire::WireType::NUM, ref, root);
        case ControlType::MIDI_PORTAL:
            return PortalControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                              exposerUuid, exposingUuid, ConnectionWire::WireType::MIDI, ref, root);
        case ControlType::SCOPE:
            return ScopeControl::deserialize(stream, uuid, parentUuid, pos, size, selected, std::move(name), showName,
                                             exposerUuid, exposingUuid, ref, root);
    }

    unreachable;
}

void Control::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);

    stream << (uint8_t) _controlType;
    GridItem::serialize(stream);
    stream << _name;
    stream << _showName;
    stream << _exposerUuid;
    stream << _exposingUuid;
}

void Control::setName(const QString &name) {
    if (name != _name) {
        _name = name;
        nameChanged.trigger(name);
    }
}

void Control::setShowName(bool showName) {
    if (showName != _showName) {
        _showName = showName;
        showNameChanged.trigger(showName);
    }
}

void Control::setExposerUuid(QUuid exposerUuid) {
    if (exposerUuid != _exposerUuid) {
        _exposerUuid = exposerUuid;
        exposerUuidChanged.trigger(exposerUuid);
    }
}

void Control::setIsActive(bool isActive) {
    if (isActive != _isActive) {
        _isActive = isActive;
        isActiveChanged.trigger(isActive);
    }
}

QPointF Control::worldPos() const {
    auto worldPos = pos() + ControlSurface::nodeToControl(_surface->node()->pos());
    return ControlSurface::controlToNode(worldPos);
}

Sequence<ModelObject*> Control::links() {
    auto expId = exposerUuid();
    return flatten(std::array<Sequence<ModelObject*>, 2> {
        staticCast<ModelObject*>(filter(root()->controls(), std::function([expId](Control *const &obj) -> bool { return obj->uuid() == expId; }))).sequence(),
        staticCast<ModelObject*>(_connections.sequence()).sequence()
    });
}

void Control::remove() {
    ModelObject::remove();
}

void Control::updateSinkPos() {
    worldPosChanged.trigger(worldPos());
}

void Control::updateExposerRemoved() {
    if (!_exposingUuid.isNull()) {
        auto baseControl = findMaybe(root()->controls(), _exposingUuid);
        if (baseControl) (*baseControl)->setExposerUuid(QUuid());
    }
}
