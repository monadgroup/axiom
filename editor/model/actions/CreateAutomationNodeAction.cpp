#include "CreateAutomationNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/AutomationNode.h"
#include "../objects/ControlSurface.h"

using namespace AxiomModel;

CreateAutomationNodeAction::CreateAutomationNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                                                       QString name, const QUuid &controlsUuid,
                                                       const QUuid &controlUuid, AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_AUTOMATION_NODE, root), uuid(uuid), parentUuid(parentUuid), pos(pos),
      name(std::move(name)), controlsUuid(controlsUuid), controlUuid(controlUuid) {
}

std::unique_ptr<CreateAutomationNodeAction> CreateAutomationNodeAction::create(const QUuid &uuid,
                                                                               const QUuid &parentUuid, QPoint pos,
                                                                               QString name, const QUuid &controlsUuid,
                                                                               const QUuid &controlUuid,
                                                                               AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateAutomationNodeAction>(uuid, parentUuid, pos, std::move(name), controlsUuid,
                                                        controlUuid, root);
}

std::unique_ptr<CreateAutomationNodeAction> CreateAutomationNodeAction::create(const QUuid &parentUuid, QPoint pos,
                                                                               QString name,
                                                                               AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, pos, std::move(name), QUuid::createUuid(), QUuid::createUuid(),
                  root);
}

void CreateAutomationNodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << parentUuid;
    stream << pos;
    stream << name;
    stream << controlsUuid;
    stream << controlUuid;
}

std::unique_ptr<CreateAutomationNodeAction> CreateAutomationNodeAction::deserialize(QDataStream &stream,
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
    QUuid controlUuid;
    stream >> controlUuid;

    return create(uuid, parentUuid, pos, std::move(name), controlsUuid, controlUuid, root);
}

bool CreateAutomationNodeAction::forward(bool) {
    root()->pool().registerObj(
        AutomationNode::create(uuid, parentUuid, pos, QSize(1, 1), false, name, controlsUuid, root()));
    root()->pool().registerObj(ControlSurface::create(controlsUuid, uuid, root()));
    root()->pool().registerObj(
        PortalControl::create(controlUuid, controlsUuid, QPoint(0, 0), QSize(2, 2), false, "", false, QUuid(), QUuid(),
                              ConnectionWire::WireType::NUM, PortalControl::PortalType::AUTOMATION, root()));
    return true;
}

bool CreateAutomationNodeAction::backward() {
    find(root()->nodes(), uuid)->remove();
    return true;
}
