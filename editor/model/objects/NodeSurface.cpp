#include "NodeSurface.h"

#include "RootSurface.h"
#include "GroupSurface.h"
#include "Node.h"
#include "Control.h"
#include "Connection.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "compiler/runtime/Surface.h"

using namespace AxiomModel;

NodeSurface::NodeSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom,
                         AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::NODE_SURFACE, uuid, parentUuid, root),
      _nodes(findChildrenWatch(root->nodes(), uuid)), _connections(findChildrenWatch(root->connections(), uuid)),
      _grid(staticCastWatch<GridItem *>(_nodes)), _pan(pan), _zoom(zoom) {
    _nodes.itemAdded.connect(this, &NodeSurface::nodeAdded);
}

std::unique_ptr<NodeSurface> NodeSurface::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                      ReferenceMapper *ref, AxiomModel::ModelRoot *root) {
    QPointF pan;
    stream >> pan;
    float zoom;
    stream >> zoom;

    if (parentUuid.isNull()) {
        return std::make_unique<RootSurface>(uuid, pan, zoom, root);
    } else {
        return std::make_unique<GroupSurface>(uuid, parentUuid, pan, zoom, root);
    }
}

void NodeSurface::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);

    stream << pan();
    stream << zoom();
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

    auto copyNodes = filter(_nodes, [](Node *const &node) {
        return node->isSelected() && node->isCopyable();
    });
    auto poolSequence = dynamicCast<ModelObject*>(pool()->sequence().sequence());
    auto copyChildren = flatten(map<Sequence<ModelObject*>, Sequence<Node*>>(copyNodes.sequence(), [poolSequence](Node *const &node) -> Sequence<ModelObject*> {
        return findDependents(poolSequence, node->uuid());
    }));
    auto copyControls = dynamicCast<Control*>(copyChildren);
    QSet<QUuid> controlUuids;
    for (const auto &control : copyControls) {
        controlUuids.insert(control->uuid());
    }

    auto copyConnections = filter(_connections, [controlUuids](Connection *const &connection) {
        return controlUuids.contains(connection->controlAUuid()) && controlUuids.contains(connection->controlBUuid());
    });

    return flatten(std::array<Sequence<ModelObject*>, 2> {
        copyChildren,
        staticCast<ModelObject*>(copyConnections).sequence()
    });
}

void NodeSurface::attachRuntime(MaximRuntime::Surface *runtime) {
    assert(!_runtime);
    _runtime = runtime;

    for (const auto &node : _nodes) {
        node->createAndAttachRuntime(runtime);
    }
}

void NodeSurface::doRuntimeUpdate() {
    for (const auto &node : nodes()) {
        node->doRuntimeUpdate();
    }
}

void NodeSurface::saveValue() {
    for (const auto &node : nodes()) {
        node->saveValue();
    }
}

void NodeSurface::restoreValue() {
    for (const auto &node : nodes()) {
        node->restoreValue();
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
        node->createAndAttachRuntime(*_runtime);
    }
}
