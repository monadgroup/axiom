#include "SetNumValueAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/NumControl.h"

using namespace AxiomModel;

SetNumValueAction::SetNumValueAction(const QUuid &uuid, NumValue beforeVal, NumValue afterVal,
                                     AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_NUM_VALUE, root), _uuid(uuid), _beforeVal(beforeVal), _afterVal(afterVal) {}

std::unique_ptr<SetNumValueAction> SetNumValueAction::create(const QUuid &uuid, NumValue beforeVal, NumValue afterVal,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<SetNumValueAction>(uuid, beforeVal, afterVal, root);
}

void SetNumValueAction::forward(bool) {
    find(AxiomCommon::dynamicCast<NumControl *>(root()->controls().sequence()), _uuid)->setValue(_afterVal);
}

void SetNumValueAction::backward() {
    find(AxiomCommon::dynamicCast<NumControl *>(root()->controls().sequence()), _uuid)->setValue(_beforeVal);
}
