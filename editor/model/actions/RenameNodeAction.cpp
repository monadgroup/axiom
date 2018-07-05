#include "RenameNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Node.h"

using namespace AxiomModel;

RenameNodeAction::RenameNodeAction(const QUuid &uuid, QString oldName, QString newName, AxiomModel::ModelRoot *root)
    : Action(ActionType::RENAME_NODE, root), uuid(uuid), oldName(std::move(oldName)), newName(std::move(newName)) {}

std::unique_ptr<RenameNodeAction> RenameNodeAction::create(const QUuid &uuid, QString oldName, QString newName,
                                                           AxiomModel::ModelRoot *root) {
    return std::make_unique<RenameNodeAction>(uuid, std::move(oldName), std::move(newName), root);
}

std::unique_ptr<RenameNodeAction> RenameNodeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QString oldName;
    stream >> oldName;
    QString newName;
    stream >> newName;

    return create(uuid, std::move(oldName), std::move(newName), root);
}

void RenameNodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << oldName;
    stream << newName;
}

void RenameNodeAction::forward(bool, MaximCompiler::Transaction *) {
    find(root()->nodes(), uuid)->setName(newName);
}

void RenameNodeAction::backward(MaximCompiler::Transaction *) {
    find(root()->nodes(), uuid)->setName(oldName);
}
