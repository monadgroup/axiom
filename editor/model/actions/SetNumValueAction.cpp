#include "SetNumValueAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/NumControl.h"

using namespace AxiomModel;

SetNumValueAction::SetNumValueAction(const QUuid &uuid, NumValue beforeVal, NumValue afterVal,
                                     AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_NUM_VALUE, root), uuid(uuid), beforeVal(beforeVal), afterVal(afterVal) {}

std::unique_ptr<SetNumValueAction> SetNumValueAction::create(const QUuid &uuid, NumValue beforeVal, NumValue afterVal,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<SetNumValueAction>(uuid, beforeVal, afterVal, root);
}

std::unique_ptr<SetNumValueAction> SetNumValueAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    NumValue beforeVal;
    stream >> beforeVal;
    NumValue afterVal;
    stream >> afterVal;

    return create(uuid, beforeVal, afterVal, root);
}

void SetNumValueAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << beforeVal;
    stream << afterVal;
}

void SetNumValueAction::forward(bool, MaximCompiler::Transaction *) {
    find<NumControl *>(root()->controls(), uuid)->setValue(afterVal);
}

void SetNumValueAction::backward(MaximCompiler::Transaction *) {
    find<NumControl *>(root()->controls(), uuid)->setValue(beforeVal);
}
