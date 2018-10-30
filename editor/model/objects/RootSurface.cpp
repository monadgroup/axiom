#include "RootSurface.h"

using namespace AxiomModel;

RootSurface::RootSurface(const QUuid &uuid, QPointF pan, float zoom, size_t nextPortalId, AxiomModel::ModelRoot *root)
    : NodeSurface(uuid, QUuid(), pan, zoom, root), _nextPortalId(nextPortalId) {}

QString RootSurface::debugName() {
    return "RootSurface";
}
