#include "NodeSurface.h"

#include "RootSurface.h"
#include "GroupSurface.h"
#include "Node.h"
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
                                                      AxiomModel::ModelRoot *root) {
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

void NodeSurface::attachRuntime(MaximRuntime::Surface *runtime) {
    assert(!_runtime);
    _runtime = runtime;

    std::cout << "Attached runtime to NodeSurface" << std::endl;

    for (const auto &node : nodes()) {
        node->createAndAttachRuntime(runtime);
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
    std::cout << "Node added, runtime = ";
    if (_runtime) std::cout << *_runtime;
    else std::cout << "false";
    std::cout << std::endl;
    if (_runtime) {
        node->createAndAttachRuntime(*_runtime);
    }
}
