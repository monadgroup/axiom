#include "MoveNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Node.h"

using namespace AxiomModel;

MoveNodeAction::MoveNodeAction(const QUuid &uuid, QPoint beforePos, QPoint afterPos, AxiomModel::ModelRoot *root)
    : Action(ActionType::MOVE_NODE, root), uuid(uuid), beforePos(beforePos), afterPos(afterPos) {
}

std::unique_ptr<MoveNodeAction> MoveNodeAction::create(const QUuid &uuid, QPoint beforePos, QPoint afterPos,
                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<MoveNodeAction>(uuid, beforePos, afterPos, root);
}

std::unique_ptr<MoveNodeAction> MoveNodeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    QPoint beforePos; stream >> beforePos;
    QPoint afterPos; stream >> afterPos;

    return create(uuid, beforePos, afterPos, root);
}

void MoveNodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << beforePos;
    stream << afterPos;
}

void MoveNodeAction::forward(bool) {
    find(root()->nodes(), uuid)->setPos(afterPos);
}

void MoveNodeAction::backward() {
    find(root()->nodes(), uuid)->setPos(beforePos);
}
