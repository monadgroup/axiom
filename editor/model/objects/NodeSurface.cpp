#include "NodeSurface.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
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
      _nodes(AxiomCommon::boxWatchSequence(findChildrenWatch(root->nodes(), uuid))),
      _connections(AxiomCommon::boxWatchSequence(findChildrenWatch(root->connections(), uuid))),
      _grid(AxiomCommon::boxWatchSequence(
                AxiomCommon::staticCastWatch<GridItem *>(AxiomCommon::refWatchSequence(&_nodes))),
            true),
      _pan(pan), _zoom(zoom) {
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

    auto copyNodes = AxiomCommon::filter(AxiomCommon::refSequence(&_nodes.sequence()),
                                         [](Node *node) { return node->isSelected() && node->isCopyable(); });
    auto poolSequence = AxiomCommon::dynamicCast<ModelObject *>(pool()->sequence().sequence());
    auto copyChildren = AxiomCommon::flatten(
        AxiomCommon::map(copyNodes, [poolSequence](Node *node) { return findDependents(poolSequence, node->uuid()); }));
    auto copyControls = AxiomCommon::dynamicCast<Control *>(copyChildren);
    QSet<QUuid> controlUuids;
    for (const auto &control : copyControls) {
        controlUuids.insert(control->uuid());
    }

    auto copyConnections =
        AxiomCommon::filter(AxiomCommon::refSequence(&_connections.sequence()), [controlUuids](Connection *connection) {
            return controlUuids.contains(connection->controlAUuid()) &&
                   controlUuids.contains(connection->controlBUuid());
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
    // flush the grid surfcaces
    _grid.tryFlush();
    _wireGrid.tryFlush();

    // todo: make this more efficient?
    auto rootControls = root()->controls();
    for (const auto &control : rootControls.sequence()) {
        if (control->surface()->node()->surface() == this) {
            control->doRuntimeUpdate();
        }
    }
    for (const auto &node : nodes().sequence()) {
        node->doRuntimeUpdate();
    }
}

void NodeSurface::remove() {
    while (!_nodes.sequence().empty()) {
        (*_nodes.sequence().begin())->remove();
    }
    while (!_connections.sequence().empty()) {
        (*_connections.sequence().begin())->remove();
    }
    ModelObject::remove();
}

void NodeSurface::nodeAdded(AxiomModel::Node *node) const {
    if (_runtime) {
        node->attachRuntime(_runtime, nullptr);
    }
}
