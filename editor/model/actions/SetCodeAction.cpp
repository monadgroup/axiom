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

void SetCodeAction::forward(bool, MaximCompiler::Transaction *transaction) {
    auto node = find<CustomNode *>(root()->nodes(), uuid);
    node->setCode(newCode);
    node->updateControls();

    if (transaction) {
        node->build(transaction);

        // todo: only build the surface if the controls changed
        node->surface()->build(transaction);
    }
}

void SetCodeAction::backward(MaximCompiler::Transaction *transaction) {
    auto node = find<CustomNode *>(root()->nodes(), uuid);
    node->setCode(oldCode);
    node->updateControls();

    if (transaction) {
        node->build(transaction);

        // todo: see above
        node->surface()->build(transaction);
    }
}
