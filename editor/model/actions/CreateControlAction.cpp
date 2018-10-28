#include "CreateControlAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/ExtractControl.h"
#include "../objects/MidiControl.h"
#include "../objects/NumControl.h"
#include "CompositeAction.h"

using namespace AxiomModel;

CreateControlAction::CreateControlAction(const QUuid &uuid, const QUuid &parentUuid, Control::ControlType type,
                                         QString name, QPoint pos, QSize size, bool isWrittenTo,
                                         AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_CONTROL, root), _uuid(uuid), _parentUuid(parentUuid), _type(type),
      _name(std::move(name)), _pos(pos), _size(size), _isWrittenTo(isWrittenTo) {}

std::unique_ptr<CreateControlAction> CreateControlAction::create(const QUuid &uuid, const QUuid &parentUuid,
                                                                 Control::ControlType type, QString name, QPoint pos,
                                                                 QSize size, bool isWrittenTo,
                                                                 AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateControlAction>(uuid, parentUuid, type, std::move(name), pos, size, isWrittenTo, root);
}

std::unique_ptr<CreateControlAction> CreateControlAction::create(const QUuid &parentUuid, Control::ControlType type,
                                                                 QString name, QPoint pos, QSize size, bool isWrittenTo,
                                                                 AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, type, std::move(name), pos, size, isWrittenTo, root);
}

std::unique_ptr<CompositeAction> CreateControlAction::create(const QUuid &parentUuid,
                                                             AxiomModel::Control::ControlType type, QString name,
                                                             bool isWrittenTo, AxiomModel::ModelRoot *root) {
    auto prepareData = Control::buildControlPrepareAction(type, parentUuid, root);
    prepareData.preActions->actions().push_back(CreateControlAction::create(
        parentUuid, type, std::move(name), prepareData.pos, prepareData.size, isWrittenTo, root));
    return std::move(prepareData.preActions);
}

void CreateControlAction::forward(bool) {
    root()->pool().registerObj(
        Control::createDefault(_type, _uuid, _parentUuid, _name, QUuid(), _pos, _size, _isWrittenTo, root()));
}

void CreateControlAction::backward() {
    find(root()->controls().sequence(), _uuid)->remove();
}
