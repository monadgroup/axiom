#include "SetShowNameAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Control.h"

using namespace AxiomModel;

SetShowNameAction::SetShowNameAction(const QUuid &uuid, bool beforeVal, bool afterVal, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_SHOW_NAME, root), _uuid(uuid), _beforeVal(beforeVal), _afterVal(afterVal) {}

std::unique_ptr<SetShowNameAction> SetShowNameAction::create(const QUuid &uuid, bool beforeVal, bool afterVal,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<SetShowNameAction>(uuid, beforeVal, afterVal, root);
}

void SetShowNameAction::forward(bool first) {
    find(root()->controls().sequence(), _uuid)->setShowName(_afterVal);
}

void SetShowNameAction::backward() {
    find(root()->controls().sequence(), _uuid)->setShowName(_beforeVal);
}
