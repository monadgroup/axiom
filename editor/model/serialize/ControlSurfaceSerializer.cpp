#include "ControlSurfaceSerializer.h"

#include "../objects/ControlSurface.h"

using namespace AxiomModel;

void ControlSurfaceSerializer::serialize(AxiomModel::ControlSurface *surface, QDataStream &stream) {}

std::unique_ptr<ControlSurface> ControlSurfaceSerializer::deserialize(QDataStream &stream, uint32_t version,
                                                                      const QUuid &uuid, const QUuid &parentUuid,
                                                                      AxiomModel::ReferenceMapper *ref,
                                                                      AxiomModel::ModelRoot *root) {
    return ControlSurface::create(uuid, parentUuid, root);
}
