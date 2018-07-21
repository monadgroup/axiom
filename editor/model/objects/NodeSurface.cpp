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
    : ModelObject(ModelType::NODE_SURFACE, uuid, parentUuid, root), _nodes(findChildrenWatch(root->nodes(), uuid)),
      _connections(findChildrenWatch(root->connections(), uuid)), _grid(staticCastWatch<GridItem *>(_nodes)), _pan(pan),
      _zoom(zoom) {
    _nodes.itemAdded.connect(this, &NodeSurface::nodeAdded);
}

void NodeSurface::setPan(QPointF pan) {
    if (pan != _pan) {
        _pan = pan;
        panChanged.trigger(pan);
    }
}

void NodeSurface::setZoom(float zoom) {
    zoom = zoom < -0.5f ? -0.5f : zoom > 0.5f ? 0.5f : zoom;
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged.trigger(zoom);
    }
}

Sequence<ModelObject *> NodeSurface::getCopyItems() const {
    // we want to copy:
    // all nodes and their children (but NOT nodes that aren't copyable!)
    // all connections that connect to controls in nodes that are selected

    auto copyNodes = filter(_nodes, [](Node *const &node) { return node->isSelected() && node->isCopyable(); });
    auto poolSequence = dynamicCast<ModelObject *>(pool()->sequence().sequence());
    auto copyChildren = flatten(map<Sequence<ModelObject *>, Sequence<Node *>>(
        copyNodes.sequence(), [poolSequence](Node *const &node) -> Sequence<ModelObject *> {
            return findDependents(poolSequence, node->uuid());
        }));
    auto copyControls = dynamicCast<Control *>(copyChildren);
    QSet<QUuid> controlUuids;
    for (const auto &control : copyControls) {
        controlUuids.insert(control->uuid());
    }

    auto copyConnections = filter(_connections, [controlUuids](Connection *const &connection) {
        return controlUuids.contains(connection->controlAUuid()) && controlUuids.contains(connection->controlBUuid());
    });

    return flatten(
        std::array<Sequence<ModelObject *>, 2>{copyChildren, staticCast<ModelObject *>(copyConnections).sequence()});
}

void NodeSurface::attachRuntime(MaximCompiler::Runtime *runtime, MaximCompiler::Transaction *transaction) {
    _runtime = runtime;
    for (const auto &node : nodes()) {
        node->attachRuntime(runtime, transaction);
    }

    if (transaction) {
        build(transaction);
    }
}

void NodeSurface::updateRuntimePointers(MaximCompiler::Runtime *runtime, void *surfacePtr) {
    for (const auto &node : nodes()) {
        node->updateRuntimePointers(runtime, surfacePtr);
    }
}

void NodeSurface::build(MaximCompiler::Transaction *transaction) {
    MaximCompiler::SurfaceMirBuilder::build(transaction, this);
}

void NodeSurface::doRuntimeUpdate() {
    // todo: make this more efficient?
    for (const auto &control : root()->controls()) {
        if (control->surface()->node()->surface() == this) {
            control->doRuntimeUpdate();
        }
    }
}

void NodeSurface::remove() {
    while (!_nodes.empty()) {
        (*_nodes.begin())->remove();
    }
    while (!_connections.empty()) {
        (*_connections.begin())->remove();
    }
    ModelObject::remove();
}

void NodeSurface::nodeAdded(AxiomModel::Node *node) const {
    if (_runtime) {
        node->attachRuntime(_runtime, nullptr);
    }
}
