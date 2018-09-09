#include "ExposeControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/Control.h"
#include "../objects/ControlSurface.h"
#include "../objects/GroupNode.h"
#include "../objects/GroupSurface.h"
#include "CompositeAction.h"

using namespace AxiomModel;

ExposeControlAction::ExposeControlAction(const QUuid &controlUuid, const QUuid &exposeUuid, QPoint pos, QSize size,
                                         AxiomModel::ModelRoot *root)
    : Action(ActionType::EXPOSE_CONTROL, root), _controlUuid(controlUuid), _exposeUuid(exposeUuid), _pos(pos),
      _size(size) {}

std::unique_ptr<ExposeControlAction> ExposeControlAction::create(const QUuid &controlUuid, const QUuid &exposeUuid,
                                                                 QPoint pos, QSize size, AxiomModel::ModelRoot *root) {
    return std::make_unique<ExposeControlAction>(controlUuid, exposeUuid, pos, size, root);
}

std::unique_ptr<ExposeControlAction> ExposeControlAction::create(const QUuid &controlUuid, QPoint pos, QSize size,
                                                                 AxiomModel::ModelRoot *root) {
    return create(controlUuid, QUuid::createUuid(), pos, size, root);
}

std::unique_ptr<CompositeAction> ExposeControlAction::create(const QUuid &controlUuid, AxiomModel::ModelRoot *root) {
    auto controlToExpose = find(root->controls(), controlUuid);
    auto controlSurface = dynamic_cast<GroupSurface *>(controlToExpose->surface()->node()->surface());
    assert(controlSurface);
    auto exposeNode = controlSurface->node();
    auto exposeSurface = *exposeNode->controls().value();

    auto prepareData = Control::buildControlPrepareAction(controlToExpose->controlType(), exposeSurface->uuid(), root);
    prepareData.preActions->actions().push_back(
        ExposeControlAction::create(controlUuid, prepareData.pos, prepareData.size, root));
    return std::move(prepareData.preActions);
}

void ExposeControlAction::forward(bool, std::vector<QUuid> &compileItems) {
    auto controlToExpose = find(root()->controls(), _controlUuid);
    controlToExpose->setExposerUuid(_exposeUuid);
    auto controlSurface = dynamic_cast<GroupSurface *>(controlToExpose->surface()->node()->surface());
    assert(controlSurface);
    auto exposeNode = controlSurface->node();
    auto exposeSurface = *exposeNode->controls().value();

    auto newControl = Control::createDefault(controlToExpose->controlType(), _exposeUuid, exposeSurface->uuid(),
                                             controlToExpose->name(), _controlUuid, _pos, _size, root());
    root()->pool().registerObj(std::move(newControl));

    compileItems.push_back(controlSurface->uuid());
    compileItems.push_back(exposeNode->surface()->uuid());
}

void ExposeControlAction::backward(std::vector<QUuid> &compileItems) {
    auto innerControl = find(root()->controls(), _controlUuid);
    auto innerSurface = innerControl->surface()->node()->surface();

    auto exposedControl = find(root()->controls(), _exposeUuid);
    auto exposeSurface = exposedControl->surface()->node()->surface();

    exposedControl->remove();

    compileItems.push_back(innerSurface->uuid());
    compileItems.push_back(exposeSurface->uuid());
}
