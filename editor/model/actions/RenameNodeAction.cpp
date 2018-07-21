#include "RenameNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Node.h"

using namespace AxiomModel;

RenameNodeAction::RenameNodeAction(const QUuid &uuid, QString oldName, QString newName, AxiomModel::ModelRoot *root)
    : Action(ActionType::RENAME_NODE, root), _uuid(uuid), _oldName(std::move(oldName)), _newName(std::move(newName)) {}

std::unique_ptr<RenameNodeAction> RenameNodeAction::create(const QUuid &uuid, QString oldName, QString newName,
                                                           AxiomModel::ModelRoot *root) {
    return std::make_unique<RenameNodeAction>(uuid, std::move(oldName), std::move(newName), root);
}

void RenameNodeAction::forward(bool, std::vector<QUuid> &) {
    find(root()->nodes(), _uuid)->setName(_newName);
}

void RenameNodeAction::backward(std::vector<QUuid> &) {
    find(root()->nodes(), _uuid)->setName(_oldName);
}
