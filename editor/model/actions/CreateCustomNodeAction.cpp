#include "CreateCustomNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/CustomNode.h"
#include "../objects/ControlSurface.h"

using namespace AxiomModel;

CreateCustomNodeAction::CreateCustomNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                               const QUuid &controlsUuid, AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_CUSTOM_NODE, root), uuid(uuid), parentUuid(parentUuid), pos(pos), name(std::move(name)),
      controlsUuid(controlsUuid) {
}

std::unique_ptr<CreateCustomNodeAction> CreateCustomNodeAction::create(const QUuid &uuid, const QUuid &parentUuid,
                                                                       QPoint pos, QString name,
                                                                       const QUuid &controlsUuid,
                                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateCustomNodeAction>(uuid, parentUuid, pos, std::move(name), controlsUuid, root);
}

std::unique_ptr<CreateCustomNodeAction> CreateCustomNodeAction::create(const QUuid &parentUuid, QPoint pos,
                                                                       QString name, AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, pos, std::move(name), QUuid::createUuid(), root);
}

std::unique_ptr<CreateCustomNodeAction> CreateCustomNodeAction::deserialize(QDataStream &stream,
                                                                            AxiomModel::ModelRoot *root) {
    QUuid uuid;
    stream >> uuid;
    QUuid parentUuid;
    stream >> parentUuid;
    QPoint pos;
    stream >> pos;
    QString name;
    stream >> name;
    QUuid controlsUuid;
    stream >> controlsUuid;

    return create(uuid, parentUuid, pos, name, controlsUuid, root);
}

void CreateCustomNodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << parentUuid;
    stream << pos;
    stream << name;
    stream << controlsUuid;
}

void CreateCustomNodeAction::forward(bool) {
    root()->pool().registerObj(
        CustomNode::create(uuid, parentUuid, pos, QSize(3, 2), false, name, controlsUuid, "", false,
                           CustomNode::minPanelHeight, root()));
    root()->pool().registerObj(ControlSurface::create(controlsUuid, uuid, root()));
}

void CreateCustomNodeAction::backward() {
    find(root()->nodes(), uuid)->remove();
}
