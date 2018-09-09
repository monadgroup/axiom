#include "Control.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../ReferenceMapper.h"
#include "Connection.h"
#include "ControlSurface.h"
#include "ExtractControl.h"
#include "GraphControl.h"
#include "GroupNode.h"
#include "GroupSurface.h"
#include "MidiControl.h"
#include "NumControl.h"
#include "PortalControl.h"
#include "editor/compiler/interface/Runtime.h"

using namespace AxiomModel;

Control::Control(AxiomModel::Control::ControlType controlType, AxiomModel::ConnectionWire::WireType wireType,
                 QUuid uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected, QString name,
                 bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid, AxiomModel::ModelRoot *root)
    : GridItem(&find(root->controlSurfaces(), parentUuid)->grid(), pos, size, selected),
      ModelObject(ModelType::CONTROL, uuid, parentUuid, root), _surface(find(root->controlSurfaces(), parentUuid)),
      _controlType(controlType), _wireType(wireType), _name(std::move(name)), _showName(showName),
      _exposerUuid(exposerUuid), _exposingUuid(exposingUuid),
      _connections(filterWatch(root->connections(),
                               std::function<bool(Connection *const &)>([uuid](Connection *const &connection) {
                                   return connection->controlAUuid() == uuid || connection->controlBUuid() == uuid;
                               }))),
      _connectedControls(
          mapFilterWatch(_connections, std::function<std::optional<QUuid>(Connection *const &)>(
                                           [uuid](Connection *const &connection) -> std::optional<QUuid> {
                                               if (connection->controlAUuid() == uuid)
                                                   return connection->controlBUuid();
                                               if (connection->controlBUuid() == uuid)
                                                   return connection->controlAUuid();
                                               return std::optional<QUuid>();
                                           }))) {
    posChanged.connect(this, &Control::updateSinkPos);
    removed.connect(this, &Control::updateExposerRemoved);
    _surface->node()->posChanged.connect(this, &Control::updateSinkPos);

    if (!_exposingUuid.isNull()) {
        findLater<Control *>(root->controls(), _exposingUuid).then([this, uuid](Control *exposing) {
            exposing->setExposerUuid(uuid);

            exposing->nameChanged.connect(this, &Control::setName);
        });
    }
}

std::unique_ptr<Control> Control::createDefault(AxiomModel::Control::ControlType type, const QUuid &uuid,
                                                const QUuid &parentUuid, const QString &name, const QUuid &exposingUuid,
                                                AxiomModel::ModelRoot *root) {
    switch (type) {
    case Control::ControlType::NUM_SCALAR:
        return NumControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(), exposingUuid,
                                  NumControl::DisplayMode::KNOB, NumControl::Channel::BOTH, {0, 0, FormType::CONTROL},
                                  root);
    case Control::ControlType::MIDI_SCALAR:
        return MidiControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(),
                                   exposingUuid, root);
    case Control::ControlType::NUM_EXTRACT:
        return ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(),
                                      exposingUuid, ConnectionWire::WireType::NUM, 0, root);
    case Control::ControlType::MIDI_EXTRACT:
        return ExtractControl::create(uuid, parentUuid, QPoint(0, 0), QSize(2, 2), false, name, true, QUuid(),
                                      exposingUuid, ConnectionWire::WireType::MIDI, 0, root);
    case Control::ControlType::GRAPH:
        return GraphControl::create(uuid, parentUuid, QPoint(0, 0), QSize(4, 4), false, name, true, QUuid(),
                                    exposingUuid, std::make_unique<GraphControlState>(), root);
    default:
        unreachable;
    }
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

Sequence<ModelObject *> Control::links() {
    auto expId = exposerUuid();
    return flatten(std::array<Sequence<ModelObject *>, 2>{
        staticCast<ModelObject *>(
            filter(root()->controls(), std::function<bool(Control *const &)>(
                                           [expId](Control *const &obj) -> bool { return obj->uuid() == expId; })))
            .sequence(),
        staticCast<ModelObject *>(_connections.sequence()).sequence()});
}

Sequence<QUuid> Control::compileLinks() {
    return oneShot(surface()->node()->parentUuid());
}

const std::optional<ControlCompileMeta> &Control::compileMeta() const {
    if (exposingUuid().isNull()) {
        return _compileMeta;
    } else {
        return find(root()->controls(), exposingUuid())->compileMeta();
    }
}

const std::optional<MaximFrontend::ControlPointers> &Control::runtimePointers() const {
    if (exposingUuid().isNull()) {
        return _runtimePointers;
    } else {
        return find(root()->controls(), exposingUuid())->runtimePointers();
    }
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
