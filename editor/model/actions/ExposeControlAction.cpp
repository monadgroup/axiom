#include "ExposeControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Control.h"
#include "../objects/ControlSurface.h"
#include "../objects/GroupNode.h"
#include "../objects/GroupSurface.h"

using namespace AxiomModel;

ExposeControlAction::ExposeControlAction(const QUuid &controlUuid, const QUuid &exposeUuid, AxiomModel::ModelRoot *root)
    : Action(ActionType::EXPOSE_CONTROL, root), controlUuid(controlUuid), exposeUuid(exposeUuid) {}

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
    QUuid controlUuid;
    stream >> controlUuid;
    QUuid exposeUuid;
    stream >> exposeUuid;

    return create(controlUuid, exposeUuid, root);
}

void ExposeControlAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << controlUuid;
    stream << exposeUuid;
}

void ExposeControlAction::forward(bool, std::vector<QUuid> &compileItems) {
    auto controlToExpose = find(root()->controls(), controlUuid);
    controlToExpose->setExposerUuid(exposeUuid);
    auto controlSurface = dynamic_cast<GroupSurface *>(controlToExpose->surface()->node()->surface());
    assert(controlSurface);
    auto exposeNode = controlSurface->node();
    auto exposeSurface = *exposeNode->controls().value();

    auto newControl = Control::createDefault(controlToExpose->controlType(), exposeUuid, exposeSurface->uuid(),
                                             controlToExpose->name(), controlUuid, root());
    root()->pool().registerObj(std::move(newControl));

    compileItems.push_back(controlSurface->uuid());
    compileItems.push_back(exposeNode->surface()->uuid());
}

void ExposeControlAction::backward(std::vector<QUuid> &compileItems) {
    auto innerControl = find(root()->controls(), controlUuid);
    auto innerSurface = innerControl->surface()->node()->surface();

    auto exposedControl = find(root()->controls(), exposeUuid);
    auto exposeSurface = exposedControl->surface()->node()->surface();

    exposedControl->remove();

    compileItems.push_back(innerSurface->uuid());
    compileItems.push_back(exposeSurface->uuid());
}
