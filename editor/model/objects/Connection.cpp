#include "Connection.h"

#include "Control.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "NodeSurface.h"

using namespace AxiomModel;

Connection::Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB,
                       AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONNECTION, uuid, parentUuid, root),
      _surface(find(root->nodeSurfaces(), parentUuid)), _controlA(find(root->controls(), controlA)),
      _controlB(find(root->controls(), controlB)), _wire(&_surface->grid(), _controlA->wireType(),
                                                         _controlA->worldPos().toPoint(), _controlB->worldPos().toPoint()) {
    _controlA->worldPosChanged.connect(&_wire, std::function([this](QPointF newPos) { _wire.setStartPos(newPos.toPoint()); }));
    _controlB->worldPosChanged.connect(&_wire, std::function([this](QPointF newPos) { _wire.setEndPos(newPos.toPoint()); }));
    _controlA->isActiveChanged.connect(&_wire, std::function([this](bool isActive) { _wire.setStartActive(isActive); }));
    _controlB->isActiveChanged.connect(&_wire, std::function([this](bool isActive) { _wire.setEndActive(isActive); }));
    _controlA->removed.connect(this, &Connection::remove);
    _controlB->removed.connect(this, &Connection::remove);
}

std::unique_ptr<Connection> Connection::create(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA,
                                               const QUuid &controlB, AxiomModel::ModelRoot *root) {
    return std::make_unique<Connection>(uuid, parentUuid, controlA, controlB, root);
}

std::unique_ptr<Connection> Connection::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    AxiomModel::ModelRoot *root) {
    QUuid controlA; stream >> controlA;
    QUuid controlB; stream >> controlB;

    return create(uuid, parentUuid, controlA, controlB, root);
}

void Connection::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);
    stream << _controlA->uuid();
    stream << _controlB->uuid();
}

void Connection::remove() {
    _wire.remove();
    ModelObject::remove();
}
