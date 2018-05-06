#include "CreateGroupNodeAction.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../objects/GroupNode.h"
#include "../objects/ControlSurface.h"
#include "../objects/GroupSurface.h"

using namespace AxiomModel;

CreateGroupNodeAction::CreateGroupNodeAction(const QUuid &uuid, const QUuid &parentUuid, QPoint pos, QSize size,
                                             bool selected, QString name, const QUuid &controlsUuid,
                                             const QUuid &innerUuid, QPointF innerPan, float innerZoom,
                                             AxiomModel::ModelRoot *root)
    : Action(ActionType::CREATE_GROUP_NODE, true, root), uuid(uuid), parentUuid(parentUuid), pos(pos), size(size),
      selected(selected), name(std::move(name)), controlsUuid(controlsUuid), innerUuid(innerUuid), innerPan(innerPan),
      innerZoom(innerZoom) {
}

std::unique_ptr<CreateGroupNodeAction> CreateGroupNodeAction::create(const QUuid &uuid, const QUuid &parentUuid,
                                                                     QPoint pos, QSize size, bool selected,
                                                                     QString name, const QUuid &controlsUuid,
                                                                     const QUuid &innerUuid, QPointF innerPan,
                                                                     float innerZoom, AxiomModel::ModelRoot *root) {
    return std::make_unique<CreateGroupNodeAction>(uuid, parentUuid, pos, size, selected, name, controlsUuid, innerUuid, innerPan, innerZoom, root);
}

std::unique_ptr<CreateGroupNodeAction> CreateGroupNodeAction::create(const QUuid &parentUuid, QPoint pos, QSize size,
                                                                     bool selected, QString name, QPointF innerPan,
                                                                     float innerZoom, AxiomModel::ModelRoot *root) {
    return create(QUuid::createUuid(), parentUuid, pos, size, selected, name, QUuid::createUuid(), QUuid::createUuid(), innerPan, innerZoom, root);
}

std::unique_ptr<CreateGroupNodeAction> CreateGroupNodeAction::deserialize(QDataStream &stream,
                                                                          AxiomModel::ModelRoot *root) {
    QUuid uuid; stream >> uuid;
    QUuid parentUuid; stream >> parentUuid;
    QPoint pos; stream >> pos;
    QSize size; stream >> size;
    bool selected; stream >> selected;
    QString name; stream >> name;
    QUuid controlsUuid; stream >> controlsUuid;
    QUuid innerUuid; stream >> innerUuid;
    QPointF innerPan; stream >> innerPan;
    float innerZoom; stream >> innerZoom;

    return create(uuid, parentUuid, pos, size, selected, name, controlsUuid, innerUuid, innerPan, innerZoom, root);
}

void CreateGroupNodeAction::serialize(QDataStream &stream) const {
    stream << uuid;
    stream << parentUuid;
    stream << pos;
    stream << size;
    stream << selected;
    stream << name;
    stream << controlsUuid;
    stream << innerUuid;
    stream << innerPan;
    stream << innerZoom;
}

void CreateGroupNodeAction::forward() const {
    root()->pool().registerObj(GroupNode::create(uuid, parentUuid, pos, size, selected, name, controlsUuid, innerUuid, root()));
    root()->pool().registerObj(ControlSurface::create(controlsUuid, uuid, root()));
    root()->pool().registerObj(GroupSurface::create(innerUuid, uuid, innerPan, innerZoom, root()));
}

void CreateGroupNodeAction::backward() const {
    find(root()->nodes(), uuid)->remove();
}
