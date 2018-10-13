#include "UnexposeControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/ControlSurface.h"
#include "../objects/Node.h"

using namespace AxiomModel;

UnexposeControlAction::UnexposeControlAction(const QUuid &controlUuid,
                                             std::unique_ptr<DeleteObjectAction> deleteExposerAction,
                                             AxiomModel::ModelRoot *root)
    : Action(ActionType::UNEXPOSE_CONTROL, root), _controlUuid(controlUuid),
      _deleteExposerAction(std::move(deleteExposerAction)) {}

std::unique_ptr<UnexposeControlAction>
    UnexposeControlAction::create(const QUuid &controlUuid, std::unique_ptr<DeleteObjectAction> deleteExposerAction,
                                  AxiomModel::ModelRoot *root) {
    return std::make_unique<UnexposeControlAction>(controlUuid, std::move(deleteExposerAction), root);
}

std::unique_ptr<UnexposeControlAction> UnexposeControlAction::create(const QUuid &controlUuid,
                                                                     AxiomModel::ModelRoot *root) {
    auto control = find(root->controls().sequence(), controlUuid);
    return create(controlUuid, DeleteObjectAction::create(control->exposerUuid(), root), root);
}

void UnexposeControlAction::forward(bool first, std::vector<QUuid> &compileItems) {
    auto control = find(root()->controls().sequence(), _controlUuid);
    compileItems.push_back(control->surface()->node()->parentUuid());

    _deleteExposerAction->forward(first, compileItems);
}

void UnexposeControlAction::backward(std::vector<QUuid> &compileItems) {
    auto control = find(root()->controls().sequence(), _controlUuid);
    compileItems.push_back(control->surface()->node()->parentUuid());

    _deleteExposerAction->backward(compileItems);
}
