#include "CreateGroupNodeAction.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GroupNode.h"
#include "../objects/ControlSurface.h"
#include "../objects/GroupSurface.h"

using namespace AxiomModel;

CreateGroupNodeAction::CreateGroupNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                             const QUuid &controlsUuid, const QUuid &innerUuid,
                                             AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_GROUP_NODE, root), uuid(uuid), parentUuid(parentUuid), pos(pos), name(std::move(name)),
      controlsUuid(controlsUuid), innerUuid(innerUuid) {
}

std::unique_ptr<CreateGroupNodeAction> CreateGroupNodeAction::create(const QUuid &uuid, const QUuid &parentUuid,
                                                                     QPoint pos, QString name,
                                                                     const QUuid &controlsUuid, const QUuid &innerUuid,
                                                                     AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateGroupNodeAction>(uuid, parentUuid, pos, std::move(name), controlsUuid, innerUuid,
                                                   root);
}

std::unique_ptr<CreateGroupNodeAction> CreateGroupNodeAction::create(const QUuid &parentUuid, QPoint pos, QString name,
                                                                     AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, pos, std::move(name), QUuid::createUuid(), QUuid::createUuid(),
                  root);
}

std::unique_ptr<CreateGroupNodeAction> CreateGroupNodeAction::deserialize(QDataStream &stream,
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
    QUuid innerUuid;
    stream >> innerUuid;

    return create(uuid, parentUuid, pos, std::move(name), controlsUuid, innerUuid, root);
}

void CreateGroupNodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << parentUuid;
    stream << pos;
    stream << name;
    stream << controlsUuid;
    stream << innerUuid;
}

bool CreateGroupNodeAction::forward(bool) {
    root()->pool().registerObj(
        GroupNode::create(uuid, parentUuid, pos, QSize(3, 2), false, name, controlsUuid, innerUuid, root()));
    root()->pool().registerObj(ControlSurface::create(controlsUuid, uuid, root()));
    root()->pool().registerObj(GroupSurface::create(innerUuid, uuid, QPoint(0, 0), 0, root()));
    return false;
}

bool CreateGroupNodeAction::backward() {
    find(root()->nodes(), uuid)->remove();
    return false;
}
