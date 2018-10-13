#include "SetCodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/CustomNode.h"
#include "../objects/NodeSurface.h"

using namespace AxiomModel;

SetCodeAction::SetCodeAction(const QUuid &uuid, QString oldCode, QString newCode,
                             std::vector<std::unique_ptr<Action>> controlActions, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_CODE, root), _uuid(uuid), _oldCode(std::move(oldCode)), _newCode(std::move(newCode)),
      _controlActions(std::move(controlActions)) {}

std::unique_ptr<SetCodeAction> SetCodeAction::create(const QUuid &uuid, QString oldCode, QString newCode,
                                                     std::vector<std::unique_ptr<Action>> controlActions,
                                                     AxiomModel::ModelRoot *root) {
    return std::make_unique<SetCodeAction>(uuid, std::move(oldCode), std::move(newCode), std::move(controlActions),
                                           root);
}

void SetCodeAction::forward(bool first, std::vector<QUuid> &compileItems) {
    auto node = find<CustomNode *>(root()->nodes().sequence(), _uuid);
    node->setCode(_newCode);

    compileItems.push_back(node->uuid());
    compileItems.push_back(node->surface()->uuid());
    for (const auto &action : _controlActions) {
        action->forward(first, compileItems);
    }
}

void SetCodeAction::backward(std::vector<QUuid> &compileItems) {
    auto node = find<CustomNode *>(root()->nodes().sequence(), _uuid);
    node->setCode(_oldCode);

    compileItems.push_back(node->uuid());
    compileItems.push_back(node->surface()->uuid());
    if (!_controlActions.empty()) {
        for (auto i = _controlActions.end() - 1; i >= _controlActions.begin(); i--) {
            (*i)->backward(compileItems);
        }
    }
}
