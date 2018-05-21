#include "SetCodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/CustomNode.h"

using namespace AxiomModel;

SetCodeAction::SetCodeAction(const QUuid &uuid, QString oldCode, QString newCode, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_CODE, root), uuid(uuid), oldCode(std::move(oldCode)), newCode(std::move(newCode)) {
}

std::unique_ptr<SetCodeAction> SetCodeAction::create(const QUuid &uuid, QString oldCode, QString newCode,
                                                     AxiomModel::ModelRoot *root) {
    return std::make_unique<SetCodeAction>(uuid, std::move(oldCode), std::move(newCode), root);
}

std::unique_ptr<SetCodeAction> SetCodeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    QString oldCode; stream >> oldCode;
    QString newCode; stream >> newCode;

    return create(uuid, std::move(oldCode), std::move(newCode), root);
}

void SetCodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << oldCode;
    stream << newCode;
}

bool SetCodeAction::forward(bool) {
    find<CustomNode*>(root()->nodes(), uuid)->setCode(newCode);
    return true;
}

bool SetCodeAction::backward() {
    find<CustomNode*>(root()->nodes(), uuid)->setCode(oldCode);
    return true;
}
