#include "CreatePortalNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/PortalNode.h"
#include "../objects/ControlSurface.h"

using namespace AxiomModel;

CreatePortalNodeAction::CreatePortalNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                               const QUuid &controlsUuid, AxiomModel::ConnectionWire::WireType wireType,
                                               AxiomModel::PortalControl::PortalType portalType,
                                               const QUuid &controlUuid, AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_PORTAL_NODE, root), uuid(uuid), parentUuid(parentUuid), pos(pos),
      name(std::move(name)), controlsUuid(controlsUuid), wireType(wireType), portalType(portalType),
      controlUuid(controlUuid) {
}

std::unique_ptr<CreatePortalNodeAction> CreatePortalNodeAction::create(const QUuid &uuid, const QUuid &parentUuid,
                                                                       QPoint pos, QString name,
                                                                       const QUuid &controlsUuid,
                                                                       AxiomModel::ConnectionWire::WireType wireType,
                                                                       AxiomModel::PortalControl::PortalType portalType,
                                                                       const QUuid &controlUuid,
                                                                       AxiomModel::ModelRoot *root) {
    return std::make_unique<CreatePortalNodeAction>(uuid, parentUuid, pos, name, controlsUuid, wireType, portalType,
                                                    controlUuid, root);
}

std::unique_ptr<CreatePortalNodeAction> CreatePortalNodeAction::create(const QUuid &parentUuid, QPoint pos,
                                                                       QString name,
                                                                       AxiomModel::ConnectionWire::WireType wireType,
                                                                       AxiomModel::PortalControl::PortalType portalType,
                                                                       AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, pos, std::move(name), QUuid::createUuid(), wireType, portalType,
                  QUuid::createUuid(), root);
}

void CreatePortalNodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << uuid;
    stream << parentUuid;
    stream << pos;
    stream << name;
    stream << controlsUuid;
    stream << (uint8_t) wireType;
    stream << (uint8_t) portalType;
    stream << controlUuid;
}

std::unique_ptr<CreatePortalNodeAction> CreatePortalNodeAction::deserialize(QDataStream &stream,
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
    uint8_t wireTypeInt;
    stream >> wireTypeInt;
    uint8_t portalTypeInt;
    stream >> portalTypeInt;
    QUuid controlUuid;
    stream >> controlUuid;

    return create(uuid, parentUuid, pos, std::move(name), controlsUuid, (ConnectionWire::WireType) wireTypeInt,
                  (PortalControl::PortalType) portalTypeInt, controlUuid, root);
}

bool CreatePortalNodeAction::forward(bool) {
    root()->pool().registerObj(
        PortalNode::create(uuid, parentUuid, pos, QSize(1, 1), false, name, controlsUuid, root()));
    root()->pool().registerObj(ControlSurface::create(controlsUuid, uuid, root()));
    root()->pool().registerObj(
        PortalControl::create(controlUuid, controlsUuid, QPoint(0, 0), QSize(2, 2), false, "", false, wireType, portalType,
                              root()));
    return false;
}

bool CreatePortalNodeAction::backward() {
    find(root()->nodes(), uuid)->remove();
    return false;
}
