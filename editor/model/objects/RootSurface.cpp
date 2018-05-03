#include "RootSurface.h"

using namespace AxiomModel;

RootSurface::RootSurface(const QUuid &uuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root)
    : Surface(uuid, QUuid(), pan, zoom, root) {
}
