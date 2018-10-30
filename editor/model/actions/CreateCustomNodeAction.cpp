#include "CreateCustomNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/ControlSurface.h"
#include "../objects/CustomNode.h"
#include "../objects/NodeSurface.h"

using namespace AxiomModel;

CreateCustomNodeAction::CreateCustomNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QString name,
                                               const QUuid &controlsUuid, AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_CUSTOM_NODE, root), _uuid(uuid), _parentUuid(parentUuid), _pos(pos),
      _name(std::move(name)), _controlsUuid(controlsUuid) {}

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

void CreateCustomNodeAction::forward(bool) {
    root()->pool().registerObj(CustomNode::create(_uuid, _parentUuid, _pos, QSize(3, 2), false, _name, _controlsUuid,
                                                  "", false, CustomNode::minPanelHeight, root()));
    root()->pool().registerObj(ControlSurface::create(_controlsUuid, _uuid, root()));
}

void CreateCustomNodeAction::backward() {
    find(root()->nodes().sequence(), _uuid)->remove();
}
