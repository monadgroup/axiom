#include "CreatePortalNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../Project.h"
#include "../objects/ControlSurface.h"
#include "../objects/NodeSurface.h"
#include "../objects/PortalNode.h"
#include "../objects/RootSurface.h"

using namespace AxiomModel;

CreatePortalNodeAction::CreatePortalNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                               const QUuid &controlsUuid, AxiomModel::ConnectionWire::WireType wireType,
                                               AxiomModel::PortalControl::PortalType portalType, uint64_t portalId,
                                               const QUuid &controlUuid, AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_PORTAL_NODE, root), _uuid(uuid), _parentUuid(parentUuid), _pos(pos),
      _name(std::move(name)), _controlsUuid(controlsUuid), _wireType(wireType), _portalType(portalType),
      _portalId(portalId), _controlUuid(controlUuid) {}

std::unique_ptr<CreatePortalNodeAction>
    CreatePortalNodeAction::create(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                   const QUuid &controlsUuid, AxiomModel::ConnectionWire::WireType wireType,
                                   AxiomModel::PortalControl::PortalType portalType, uint64_t portalId,
                                   const QUuid &controlUuid, AxiomModel::ModelRoot *root) {
    return std::make_unique<CreatePortalNodeAction>(uuid, parentUuid, pos, name, controlsUuid, wireType, portalType,
                                                    portalId, controlUuid, root);
}

std::unique_ptr<CreatePortalNodeAction> CreatePortalNodeAction::create(const QUuid &parentUuid, QPoint pos,
                                                                       QString name,
                                                                       AxiomModel::ConnectionWire::WireType wireType,
                                                                       AxiomModel::PortalControl::PortalType portalType,
                                                                       AxiomModel::ModelRoot *root) {
    auto portalId = 0; // todo: root->project()->rootSurface()->takePortalId();
    return create(QUuid::createUuid(), parentUuid, pos, std::move(name), QUuid::createUuid(), wireType, portalType,
                  portalId, QUuid::createUuid(), root);
}

void CreatePortalNodeAction::forward(bool, std::vector<QUuid> &compileItems) {
    root()->pool().registerObj(
        PortalNode::create(_uuid, _parentUuid, _pos, QSize(1, 1), false, _name, _controlsUuid, root()));
    root()->pool().registerObj(ControlSurface::create(_controlsUuid, _uuid, root()));
    root()->pool().registerObj(PortalControl::create(_controlUuid, _controlsUuid, QPoint(0, 0), QSize(2, 2), false, "",
                                                     false, QUuid(), QUuid(), _wireType, _portalType, _portalId,
                                                     root()));

    compileItems.push_back(_parentUuid);
}

void CreatePortalNodeAction::backward(std::vector<QUuid> &compileItems) {
    find(root()->nodes(), _uuid)->remove();

    compileItems.push_back(_parentUuid);
}
