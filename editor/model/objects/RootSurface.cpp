#include "RootSurface.h"

using namespace AxiomModel;

RootSurface::RootSurface(const QUuid &uuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root)
    : NodeSurface(uuid, QUuid(), pan, zoom, root) {
}
