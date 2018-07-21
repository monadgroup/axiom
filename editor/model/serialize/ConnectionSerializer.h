#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class ModelRoot;
    class Connection;
    class ReferenceMapper;

    namespace ConnectionSerializer {
        void serialize(Connection *connection, QDataStream &stream);

        std::unique_ptr<Connection> deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                const QUuid &parentUuid, ReferenceMapper *ref, ModelRoot *root);
    }
}
