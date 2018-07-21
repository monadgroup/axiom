#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class ModelRoot;
    class ControlSurface;
    class ReferenceMapper;

    namespace ControlSurfaceSerializer {
        void serialize(ControlSurface *surface, QDataStream &stream);

        std::unique_ptr<ControlSurface> deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                    const QUuid &parentUuid, ReferenceMapper *ref, ModelRoot *root);
    }
}
