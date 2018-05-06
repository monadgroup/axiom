#include "NodeSurface.h"

#include <QtCore/QDataStream>

#include "RootSurface.h"
#include "GroupSurface.h"
#include "Node.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

NodeSurface::NodeSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::NODE_SURFACE, uuid, parentUuid, root),
      _nodes(filterChildren(root->nodes(), uuid)), _grid(staticCast<GridItem*>(_nodes)), _pan(pan), _zoom(zoom) {
}

std::unique_ptr<NodeSurface> NodeSurface::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                              AxiomModel::ModelRoot *root) {
    QPointF pan; stream >> pan;
    float zoom; stream >> zoom;

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
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged.trigger(zoom);
    }
}

void NodeSurface::remove() {
    for (const auto &node : _nodes) {
        node->remove();
    }
    ModelObject::remove();
}
