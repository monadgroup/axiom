#include "Control.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../ReferenceMapper.h"
#include "../actions/CompositeAction.h"
#include "../actions/GridItemSizeAction.h"
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
                 QSize minSize, QUuid uuid, const QUuid &parentUuid, QPoint pos, QSize size, bool selected,
                 QString name, bool showName, const QUuid &exposerUuid, const QUuid &exposingUuid,
                 AxiomModel::ModelRoot *root)
    : GridItem(&find(root->controlSurfaces(), parentUuid)->grid(), pos, size, minSize, selected),
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

QSize Control::getDefaultSize(AxiomModel::Control::ControlType controlType) {
    switch (controlType) {
    case ControlType::NUM_SCALAR:
    case ControlType::MIDI_SCALAR:
    case ControlType::NUM_EXTRACT:
    case ControlType::MIDI_EXTRACT:
    case ControlType::NUM_PORTAL:
    case ControlType::MIDI_PORTAL:
        return QSize(2, 2);
    case ControlType::GRAPH:
        return QSize(6, 4);
    }
    unreachable;
}

std::unique_ptr<Control> Control::createDefault(AxiomModel::Control::ControlType type, const QUuid &uuid,
                                                const QUuid &parentUuid, const QString &name, const QUuid &exposingUuid,
                                                QPoint pos, QSize size, AxiomModel::ModelRoot *root) {
    switch (type) {
    case Control::ControlType::NUM_SCALAR:
        return NumControl::create(uuid, parentUuid, pos, size, false, name, true, QUuid(), exposingUuid,
                                  NumControl::DisplayMode::KNOB, NumControl::Channel::BOTH, {0, 0, FormType::CONTROL},
                                  root);
    case Control::ControlType::MIDI_SCALAR:
        return MidiControl::create(uuid, parentUuid, pos, size, false, name, true, QUuid(), exposingUuid, root);
    case Control::ControlType::NUM_EXTRACT:
        return ExtractControl::create(uuid, parentUuid, pos, size, false, name, true, QUuid(), exposingUuid,
                                      ConnectionWire::WireType::NUM, 0, root);
    case Control::ControlType::MIDI_EXTRACT:
        return ExtractControl::create(uuid, parentUuid, pos, size, false, name, true, QUuid(), exposingUuid,
                                      ConnectionWire::WireType::MIDI, 0, root);
    case Control::ControlType::GRAPH:
        return GraphControl::create(uuid, parentUuid, pos, size, false, name, true, QUuid(), exposingUuid,
                                    std::make_unique<GraphControlState>(), root);
    default:
        unreachable;
    }
}

ControlPrepare Control::buildControlPrepareAction(AxiomModel::Control::ControlType type, const QUuid &parentUuid,
                                                  AxiomModel::ModelRoot *root) {
    auto controlSurface = find<ControlSurface *>(root->pool().sequence(), parentUuid);
    auto node = controlSurface->node();
    auto controlSurfaceSize = ControlSurface::nodeToControl(node->size());

    auto controlSize = getDefaultSize(type);

    // if the control has a header visible and there's enough room, place it below the header
    auto placePos = QPoint(0, 0);
    if (!controlSurface->controlsOnTopRow() && controlSurfaceSize.height() >= controlSize.height() + 1) {
        placePos.setY(1);
    }

    bool findSuccess;
    auto nearestPos = controlSurface->grid().grid().findNearestAvailable(placePos, controlSize, nullptr, &findSuccess);

    if (findSuccess) {
        return {nearestPos, controlSize, CompositeAction::create({}, root)};
    } else {
        // no space available, temporarily disable size restriction
        auto oldMaxSize = controlSurface->grid().grid().maxRect;
        controlSurface->grid().grid().maxRect = QPoint(INT_MAX, INT_MAX);

        nearestPos = controlSurface->grid().grid().findNearestAvailable(placePos, controlSize);
        auto controlBottomRight = nearestPos + QPoint(controlSize.width(), controlSize.height());
        auto newNodeSize =
            ControlSurface::controlToNodeCeil(QSize(qMax(controlSurfaceSize.width(), controlBottomRight.x()),
                                                    qMax(controlSurfaceSize.height(), controlBottomRight.y())));

        // find an available position for the node on the main surface, since we're resizing it
        auto newNodePos = node->parentSurface->grid().findNearestAvailable(node->pos(), newNodeSize, node);

        // reset the max size
        controlSurface->grid().grid().maxRect = oldMaxSize;

        auto compositeAction = CompositeAction::create({}, root);
        compositeAction->actions().push_back(
            GridItemSizeAction::create(node->uuid(), node->rect(), QRect(newNodePos, newNodeSize), root));

        return {nearestPos, controlSize, std::move(compositeAction)};
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
