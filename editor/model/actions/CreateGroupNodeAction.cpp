#include "CreateGroupNodeAction.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/ControlSurface.h"
#include "../objects/GroupNode.h"
#include "../objects/GroupSurface.h"

using namespace AxiomModel;

CreateGroupNodeAction::CreateGroupNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                             const QUuid &controlsUuid, const QUuid &innerUuid,
                                             AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_GROUP_NODE, root), _uuid(uuid), _parentUuid(parentUuid), _pos(pos),
      _name(std::move(name)), _controlsUuid(controlsUuid), _innerUuid(innerUuid) {}

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

void CreateGroupNodeAction::forward(bool, std::vector<QUuid> &compileItems) {
    root()->pool().registerObj(
        GroupNode::create(_uuid, _parentUuid, _pos, QSize(3, 2), false, _name, _controlsUuid, _innerUuid, root()));
    root()->pool().registerObj(ControlSurface::create(_controlsUuid, _uuid, root()));
    root()->pool().registerObj(GroupSurface::create(_innerUuid, _uuid, QPoint(0, 0), 0, root()));

    compileItems.push_back(_innerUuid);
    compileItems.push_back(_parentUuid);
}

void CreateGroupNodeAction::backward(std::vector<QUuid> &compileItems) {
    find(root()->nodes(), _uuid)->remove();

    compileItems.push_back(_parentUuid);
}
