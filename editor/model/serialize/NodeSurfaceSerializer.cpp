#include "NodeSurfaceSerializer.h"

#include "../objects/GroupSurface.h"
#include "../objects/NodeSurface.h"
#include "../objects/RootSurface.h"

using namespace AxiomModel;

void NodeSurfaceSerializer::serialize(AxiomModel::NodeSurface *surface, QDataStream &stream) {
    stream << surface->pan();
    stream << surface->zoom();

    if (auto rootSurface = dynamic_cast<RootSurface *>(surface)) {
        stream << rootSurface->nextPortalId();
    }
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
        // unique portal IDs were added in 0.4.0, corresponding to schema version 5
        uint64_t nextPortalId = 0;
        if (version >= 5) {
            stream >> nextPortalId;
        }

        return std::make_unique<RootSurface>(uuid, pan, zoom, nextPortalId, root);
    } else {
        return std::make_unique<GroupSurface>(uuid, parentUuid, pan, zoom, root);
    }
}
