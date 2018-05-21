#include "ExposeControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Control.h"
#include "../objects/ControlSurface.h"
#include "../objects/GroupNode.h"
#include "../objects/GroupSurface.h"

using namespace AxiomModel;

ExposeControlAction::ExposeControlAction(const QUuid &controlUuid, const QUuid &exposeUuid,
                                         AxiomModel::ModelRoot *root)
    : Action(ActionType::EXPOSE_CONTROL, root), controlUuid(controlUuid), exposeUuid(exposeUuid) {
}

std::unique_ptr<ExposeControlAction> ExposeControlAction::create(const QUuid &controlUuid, const QUuid &exposeUuid,
                                                                 AxiomModel::ModelRoot *root) {
    return std::make_unique<ExposeControlAction>(controlUuid, exposeUuid, root);
}

std::unique_ptr<ExposeControlAction> ExposeControlAction::create(const QUuid &controlUuid,
                                                                 AxiomModel::ModelRoot *root) {
    return create(controlUuid, QUuid::createUuid(), root);
}

std::unique_ptr<ExposeControlAction> ExposeControlAction::deserialize(QDataStream &stream,
                                                                      AxiomModel::ModelRoot *root) {
    QUuid controlUuid; stream >> controlUuid;
    QUuid exposeUuid; stream >> exposeUuid;

    return create(controlUuid, exposeUuid, root);
}

void ExposeControlAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << controlUuid;
    stream << exposeUuid;
}

bool ExposeControlAction::forward(bool) {
    auto controlToExpose = find(root()->controls(), controlUuid);
    controlToExpose->setExposerUuid(exposeUuid);
    auto controlSurface = dynamic_cast<GroupSurface*>(controlToExpose->surface()->node()->surface());
    assert(controlSurface);
    auto exposeNode = controlSurface->node();
    auto exposeSurface = *exposeNode->controls().value();

    auto newControl = Control::createDefault(controlToExpose->controlType(), exposeUuid, exposeSurface->uuid(), controlToExpose->name(), controlUuid, root());
    root()->pool().registerObj(std::move(newControl));
    return true;
}

bool ExposeControlAction::backward() {
    find(root()->controls(), exposeUuid)->remove();
    return true;
}
