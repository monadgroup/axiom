#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class ModelRoot;
    class NodeSurface;
    class ReferenceMapper;

    namespace NodeSurfaceSerializer {
        void serialize(NodeSurface *surface, QDataStream &stream);

        std::unique_ptr<NodeSurface> deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                 const QUuid &parentUuid, ReferenceMapper *ref, ModelRoot *root,
                                                 bool isLibrary);
    }
}
