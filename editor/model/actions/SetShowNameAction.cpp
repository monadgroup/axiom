#include "SetShowNameAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Control.h"

using namespace AxiomModel;

SetShowNameAction::SetShowNameAction(const QUuid &uuid, bool beforeVal, bool afterVal, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_SHOW_NAME, root), uuid(uuid), beforeVal(beforeVal), afterVal(afterVal) {
}

std::unique_ptr<SetShowNameAction> SetShowNameAction::create(const QUuid &uuid, bool beforeVal, bool afterVal,
                                                             AxiomModel::ModelRoot *root) {
    return std::make_unique<SetShowNameAction>(uuid, beforeVal, afterVal, root);
}

std::unique_ptr<SetShowNameAction> SetShowNameAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    bool beforeVal; stream >> beforeVal;
    bool afterVal; stream >> afterVal;

    return create(uuid, beforeVal, afterVal, root);
}

void SetShowNameAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << beforeVal;
    stream << afterVal;
}

bool SetShowNameAction::forward(bool first) {
    find(root()->controls(), uuid)->setShowName(afterVal);
    return false;
}

bool SetShowNameAction::backward() {
    find(root()->controls(), uuid)->setShowName(beforeVal);
    return false;
}
