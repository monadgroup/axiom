#include "SetCodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/CustomNode.h"
#include "../objects/NodeSurface.h"

using namespace AxiomModel;

SetCodeAction::SetCodeAction(const QUuid &uuid, QString oldCode, QString newCode,
                             std::unique_ptr<CompositeAction> controlActions, AxiomModel::ModelRoot *root)
    : Action(ActionType::SET_CODE, root), uuid(uuid), oldCode(std::move(oldCode)), newCode(std::move(newCode)),
      _controlActions(std::move(controlActions)) {}

std::unique_ptr<SetCodeAction> SetCodeAction::create(const QUuid &uuid, QString oldCode, QString newCode,
                                                     std::unique_ptr<CompositeAction> controlActions,
                                                     AxiomModel::ModelRoot *root) {
    return std::make_unique<SetCodeAction>(uuid, std::move(oldCode), std::move(newCode), std::move(controlActions),
                                           root);
}

std::unique_ptr<SetCodeAction> SetCodeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QString oldCode;
    stream >> oldCode;
    QString newCode;
    stream >> newCode;

    auto controlActions = CompositeAction::deserialize(stream, root);

    return create(uuid, std::move(oldCode), std::move(newCode), std::move(controlActions), root);
}

void SetCodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << oldCode;
    stream << newCode;
    _controlActions->serialize(stream);
}

void SetCodeAction::forward(bool first, std::vector<QUuid> &compileItems) {
    auto node = find<CustomNode *>(root()->nodes(), uuid);
    node->setCode(newCode);

    compileItems.push_back(node->uuid());
    compileItems.push_back(node->surface()->uuid());
    _controlActions->forward(first, compileItems);
}

void SetCodeAction::backward(std::vector<QUuid> &compileItems) {
    auto node = find<CustomNode *>(root()->nodes(), uuid);
    node->setCode(oldCode);

    compileItems.push_back(node->uuid());
    compileItems.push_back(node->surface()->uuid());
    _controlActions->backward(compileItems);
}
