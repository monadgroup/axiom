#include "SetCodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/CustomNode.h"
#include "../objects/NodeSurface.h"

using namespace AxiomModel;

SetCodeAction::SetCodeAction(const QUuid &uuid, QString oldCode, QString newCode, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_CODE, root), uuid(uuid), oldCode(std::move(oldCode)), newCode(std::move(newCode)) {}

std::unique_ptr<SetCodeAction> SetCodeAction::create(const QUuid &uuid, QString oldCode, QString newCode,
                                                     AxiomModel::ModelRoot *root) {
    return std::make_unique<SetCodeAction>(uuid, std::move(oldCode), std::move(newCode), root);
}

std::unique_ptr<SetCodeAction> SetCodeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QString oldCode;
    stream >> oldCode;
    QString newCode;
    stream >> newCode;

    return create(uuid, std::move(oldCode), std::move(newCode), root);
}

void SetCodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << oldCode;
    stream << newCode;
}

void SetCodeAction::forward(bool, std::vector<QUuid> &compileItems) {
    auto node = find<CustomNode *>(root()->nodes(), uuid);
    node->setCode(newCode);

    compileItems.push_back(node->uuid());
    compileItems.push_back(node->surface()->uuid());
}

void SetCodeAction::backward(std::vector<QUuid> &compileItems) {
    auto node = find<CustomNode *>(root()->nodes(), uuid);
    node->setCode(oldCode);

    compileItems.push_back(node->uuid());
    compileItems.push_back(node->surface()->uuid());
}
