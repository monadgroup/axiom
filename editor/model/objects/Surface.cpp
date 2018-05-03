#include "Surface.h"

#include <QtCore/QDataStream>

#include "RootSurface.h"
#include "GroupSurface.h"
#include "../ModelRoot.h"

using namespace AxiomModel;

Surface::Surface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::SURFACE, uuid, parentUuid, root),
        _nodes(filterChildren(root->nodes(), uuid)), _grid(staticCast<GridItem*>(_nodes)), _pan(pan), _zoom(zoom) {
}

std::unique_ptr<Surface> Surface::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                              AxiomModel::ModelRoot *root) {
    QPointF pan; stream >> pan;
    float zoom; stream >> zoom;

    if (parentUuid.isNull()) {
        return std::make_unique<RootSurface>(uuid, pan, zoom, root);
    } else {
        return std::make_unique<GroupSurface>(uuid, parentUuid, pan, zoom, root);
    }
}

void Surface::serialize(QDataStream &stream) const {
    stream << pan();
    stream << zoom();
}

void Surface::setPan(QPointF pan) {
    if (pan != _pan) {
        _pan = pan;
        panChanged.emit(pan);
    }
}

void Surface::setZoom(float zoom) {
    if (zoom != _zoom) {
        _zoom = zoom;
        zoomChanged.emit(zoom);
    }
}
