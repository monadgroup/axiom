#include "NodeSurfaceSerializer.h"

#include "../objects/GroupSurface.h"
#include "../objects/NodeSurface.h"
#include "../objects/RootSurface.h"

using namespace AxiomModel;

void NodeSurfaceSerializer::serialize(AxiomModel::NodeSurface *surface, QDataStream &stream) {
    stream << surface->pan();
    stream << surface->zoom();
}

std::unique_ptr<NodeSurface> NodeSurfaceSerializer::deserialize(QDataStream &stream, uint32_t version,
                                                                const QUuid &uuid, const QUuid &parentUuid,
                                                                AxiomModel::ReferenceMapper *ref,
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
