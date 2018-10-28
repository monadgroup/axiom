#include "SetNumModeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"

using namespace AxiomModel;

SetNumModeAction::SetNumModeAction(const QUuid &uuid, AxiomModel::NumControl::DisplayMode beforeMode,
                                   AxiomModel::NumControl::DisplayMode afterMode, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_NUM_MODE, root), _uuid(uuid), _beforeMode(beforeMode), _afterMode(afterMode) {}

std::unique_ptr<SetNumModeAction> SetNumModeAction::create(const QUuid &uuid,
                                                           AxiomModel::NumControl::DisplayMode beforeMode,
                                                           AxiomModel::NumControl::DisplayMode afterMode,
                                                           AxiomModel::ModelRoot *root) {
    return std::make_unique<SetNumModeAction>(uuid, beforeMode, afterMode, root);
}

void SetNumModeAction::forward(bool) {
    find(AxiomCommon::dynamicCast<NumControl *>(root()->controls().sequence()), _uuid)->setDisplayMode(_afterMode);
}

void SetNumModeAction::backward() {
    find(AxiomCommon::dynamicCast<NumControl *>(root()->controls().sequence()), _uuid)->setDisplayMode(_beforeMode);
}
