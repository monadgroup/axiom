#include "RenameControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Control.h"

using namespace AxiomModel;

RenameControlAction::RenameControlAction(const QUuid &uuid, QString oldName, QString newName,
                                         AxiomModel::ModelRoot *root)
    : Action(ActionType::RENAME_CONTROL, root), _uuid(uuid), _oldName(std::move(oldName)),
      _newName(std::move(newName)) {}

std::unique_ptr<RenameControlAction> RenameControlAction::create(const QUuid &uuid, QString oldName, QString newName,
                                                                 AxiomModel::ModelRoot *root) {
    return std::make_unique<RenameControlAction>(uuid, std::move(oldName), std::move(newName), root);
}

void RenameControlAction::forward(bool, std::vector<QUuid> &compileItems) {
    find(root()->controls().sequence(), _uuid)->setName(_newName);
}

void RenameControlAction::backward(std::vector<QUuid> &compileItems) {
    find(root()->controls().sequence(), _uuid)->setName(_oldName);
}
