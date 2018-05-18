#include "SetNumValueAction.h"

#include "../ValueWriters.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/NumControl.h"

using namespace AxiomModel;

SetNumValueAction::SetNumValueAction(const QUuid &uuid, MaximRuntime::NumValue beforeVal,
                                     MaximRuntime::NumValue afterVal, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_NUM_VALUE, root), uuid(uuid), beforeVal(beforeVal), afterVal(afterVal) {
}

std::unique_ptr<SetNumValueAction> SetNumValueAction::create(const QUuid &uuid, MaximRuntime::NumValue beforeVal,
                                                             MaximRuntime::NumValue afterVal,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<SetNumValueAction>(uuid, beforeVal, afterVal, root);
}

std::unique_ptr<SetNumValueAction> SetNumValueAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    MaximRuntime::NumValue beforeVal; stream >> beforeVal;
    MaximRuntime::NumValue afterVal; stream >> afterVal;

    return create(uuid, beforeVal, afterVal, root);
}

void SetNumValueAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << beforeVal;
    stream << afterVal;
}

bool SetNumValueAction::forward(bool first) {
    find<NumControl*>(root()->controls(), uuid)->setValue(afterVal);
    return false;
}

bool SetNumValueAction::backward() {
    find<NumControl*>(root()->controls(), uuid)->setValue(beforeVal);
    return false;
}
