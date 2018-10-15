#include "NodeSurface.h"

#include "../ModelRoot.h"
#include "Connection.h"
#include "ControlSurface.h"
#include "GroupSurface.h"
#include "Node.h"
#include "RootSurface.h"
#include "editor/compiler/SurfaceMirBuilder.h"
#include "editor/compiler/interface/Runtime.h"

using namespace AxiomModel;

NodeSurface::NodeSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                         AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::NODE_SURFACE, uuid, parentUuid, root),
      _nodes(cacheSequence(findChildrenWatch(root->nodes(), uuid))),
      _connections(cacheSequence(findChildrenWatch(root->connections(), uuid))),
      _grid(AxiomCommon::boxWatchSequence(AxiomCommon::staticCastWatch<GridItem *>(_nodes.asRef())), true), _pan(pan),
      _zoom(zoom) {
    _nodes.events().itemAdded().connect(this, &NodeSurface::nodeAdded);
}

void NodeSurface::setPan(QPointF pan) {
    if (pan != _pan) {
        _pan = pan;
        panChanged(pan);
    }
}

void NodeSurface::setZoom(float zoom) {
    zoom = zoom < -0.5f ? -0.5f : zoom > 0.5f ? 0.5f : zoom;
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged(zoom);
    }
}

AxiomCommon::BoxedSequence<ModelObject *> NodeSurface::getCopyItems() {
    // we want to copy:
    // all nodes and their children (but NOT nodes that aren't copyable!)
    // all connections that connect to controls in nodes that are selected

    auto copyNodes =
        AxiomCommon::filter(_nodes.sequence(), [](Node *node) { return node->isSelected() && node->isCopyable(); });
    auto poolSequence = AxiomCommon::dynamicCast<ModelObject *>(pool()->sequence().sequence());
    auto copyChildren = AxiomCommon::flatten(
        AxiomCommon::map(copyNodes, [poolSequence](Node *node) { return findDependents(poolSequence, node->uuid()); }));
    auto copyControls = AxiomCommon::dynamicCast<Control *>(copyChildren);
    QSet<QUuid> controlUuids;
    for (const auto &control : copyControls) {
        controlUuids.insert(control->uuid());
    }

    auto copyConnections = AxiomCommon::filter(_connections.sequence(), [controlUuids](Connection *connection) {
        return controlUuids.contains(connection->controlAUuid()) && controlUuids.contains(connection->controlBUuid());
    });

    return AxiomCommon::boxSequence(AxiomCommon::flatten(std::array<AxiomCommon::BoxedSequence<ModelObject *>, 2>{
        AxiomCommon::boxSequence(std::move(copyChildren)),
        AxiomCommon::boxSequence(AxiomCommon::staticCast<ModelObject *>(copyConnections))}));
}

void NodeSurface::attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) {
    _runtime = runtime;
    for (const auto &node : nodes().sequence()) {
        node->attachRuntime(runtime, transaction);
    }

    if (transaction) {
        build(transaction);
    }
}

void NodeSurface::updateRuntimePointers(MaximCompiler::Runtime *runtime, void *surfacePtr) {
    for (const auto &node : nodes().sequence()) {
        node->updateRuntimePointers(runtime, surfacePtr);
    }
}

void NodeSurface::build(MaximCompiler::Transaction *transaction) {
    MaximCompiler::SurfaceMirBuilder::build(transaction, this);
}

void NodeSurface::doRuntimeUpdate() {
    // flush the grid surfaces
    _grid.tryFlush();
    _wireGrid.tryFlush();

    for (const auto &node : nodes().sequence()) {
        if (auto controls = node->controls().value()) {
            for (const auto &control : (*controls)->controls().sequence()) {
                control->doRuntimeUpdate();
            }
        }
        node->doRuntimeUpdate();
    }
}

void NodeSurface::remove() {
    auto nodes = findChildren(root()->nodes().sequence(), uuid());
    while (!nodes.empty()) {
        (*nodes.begin())->remove();
    }
    auto connections = findChildren(root()->connections().sequence(), uuid());
    while (!connections.empty()) {
        (*connections.begin())->remove();
    }
    ModelObject::remove();
}

void NodeSurface::nodeAdded(AxiomModel::Node *node) const {
    if (_runtime) {
        node->attachRuntime(_runtime, nullptr);
    }
}
