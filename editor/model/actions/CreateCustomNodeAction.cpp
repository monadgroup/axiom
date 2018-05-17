#include "CreateCustomNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/CustomNode.h"
#include "../objects/ControlSurface.h"

using namespace AxiomModel;

CreateCustomNodeAction::CreateCustomNodeAction(const QUuid &parentId, const QUuid &id, QPoint pos, QSize size,
                                               bool selected, QString name, const QUuid &controlsId, QString code,
                                               AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_CUSTOM_NODE, root), parentId(parentId), id(id), pos(pos), size(size),
      selected(selected), name(std::move(name)), controlsId(controlsId), code(std::move(code)) {

}

std::unique_ptr<CreateCustomNodeAction> CreateCustomNodeAction::create(const QUuid &parentId, const QUuid &id,
                                                                       QPoint pos, QSize size, bool selected,
                                                                       QString name, const QUuid &controlsUuid,
                                                                       QString code, AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateCustomNodeAction>(parentId, id, pos, size, selected, name, controlsUuid, code, root);
}

std::unique_ptr<CreateCustomNodeAction> CreateCustomNodeAction::create(const QUuid &parentId, QPoint pos, QSize size,
                                                                       bool selected, QString name, QString code,
                                                                       AxiomModel::ModelRoot *root) {
    return create(parentId, QUuid::createUuid(), pos, size, selected, name, QUuid::createUuid(), code, root);
}

std::unique_ptr<CreateCustomNodeAction> CreateCustomNodeAction::deserialize(QDataStream &stream, AxiomModel::ModelRoot *root) {
    QUuid parentId; stream >> parentId;
    QUuid id; stream >> id;
    QPoint pos; stream >> pos;
    QSize size; stream >> size;
    bool selected; stream >> selected;
    QString name; stream >> name;
    QUuid controlsId; stream >> controlsId;
    QString code; stream >> code;

    return create(parentId, id, pos, size, selected, name, controlsId, code, root);
}

void CreateCustomNodeAction::serialize(QDataStream &stream) const {
    Action::serialize(stream);

    stream << parentId;
    stream << id;
    stream << pos;
    stream << size;
    stream << selected;
    stream << name;
    stream << controlsId;
    stream << code;
}

void CreateCustomNodeAction::forward(bool) {
    root()->pool().registerObj(CustomNode::create(id, parentId, pos, size, selected, name, controlsId, code, root()));
    root()->pool().registerObj(ControlSurface::create(controlsId, id, root()));
}

void CreateCustomNodeAction::backward() {
    find(root()->nodes(), id)->remove();
}
