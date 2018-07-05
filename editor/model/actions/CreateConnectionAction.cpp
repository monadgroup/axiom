#include "CreateConnectionAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Connection.h"
#include "../objects/NodeSurface.h"

using namespace AxiomModel;

CreateConnectionAction::CreateConnectionAction(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA,
                                               const QUuid &controlB, AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_CONNECTION, root), uuid(uuid), parentUuid(parentUuid), controlA(controlA),
      controlB(controlB) {}

std::unique_ptr<CreateConnectionAction> CreateConnectionAction::create(const QUuid &uuid, const QUuid &parentUuid,
                                                                       const QUuid &controlA, const QUuid &controlB,
                                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateConnectionAction>(uuid, parentUuid, controlA, controlB, root);
}

std::unique_ptr<CreateConnectionAction> CreateConnectionAction::create(const QUuid &parentUuid, const QUuid &controlA,
                                                                       const QUuid &controlB,
                                                                       AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, controlA, controlB, root);
}

std::unique_ptr<CreateConnectionAction> CreateConnectionAction::deserialize(QDataStream &stream,
                                                                            AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    QUuid controlA;
    stream >> controlA;
    QUuid controlB;
    stream >> controlB;

    return std::make_unique<CreateConnectionAction>(uuid, parentUuid, controlA, controlB, root);
}

void CreateConnectionAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);
    stream << uuid;
    stream << parentUuid;
    stream << controlA;
    stream << controlB;
}

void CreateConnectionAction::forward(bool, MaximCompiler::Transaction *transaction) {
    root()->pool().registerObj(Connection::create(uuid, parentUuid, controlA, controlB, root()));

    if (transaction) {
        find(root()->nodeSurfaces(), parentUuid)->build(transaction);
    }
}

void CreateConnectionAction::backward(MaximCompiler::Transaction *transaction) {
    find(root()->connections(), uuid)->remove();

    if (transaction) {
        find(root()->nodeSurfaces(), parentUuid)->build(transaction);
    }
}
