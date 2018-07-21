#include "ConnectionSerializer.h"

#include "../ReferenceMapper.h"
#include "../objects/Connection.h"

using namespace AxiomModel;

void ConnectionSerializer::serialize(AxiomModel::Connection *connection, QDataStream &stream) {
    stream << connection->controlAUuid();
    stream << connection->controlBUuid();
}

std::unique_ptr<Connection> ConnectionSerializer::deserialize(QDataStream &stream, uint32_t version, const QUuid &uuid,
                                                              const QUuid &parentUuid, AxiomModel::ReferenceMapper *ref,
                                                              AxiomModel::ModelRoot *root) {
    QUuid controlA;
    stream >> controlA;
    controlA = ref->mapUuid(controlA);
    QUuid controlB;
    stream >> controlB;
    controlB = ref->mapUuid(controlB);

    return Connection::create(uuid, parentUuid, controlA, controlB, root);
}
