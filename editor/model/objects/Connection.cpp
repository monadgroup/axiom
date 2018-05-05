#include "Connection.h"

#include "Control.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "NodeSurface.h"

using namespace AxiomModel;

Connection::Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB,
                       AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONNECTION, uuid, parentUuid, root),
      _surface(find(root->nodeSurfaces(), parentUuid)), _controlAUuid(controlA),
      _controlA(findLater<Control*>(root->controls(), controlA)), _controlBUuid(controlB), _controlB(findLater<Control*>(root->controls(), controlB)),
      _wire(&_surface->grid(), QPoint(0, 0), QPoint(0, 0))  {
    _controlA.then([this](Control *control) {
        control->worldPosChanged.listen(&_wire, [this](QPointF newPos) { _wire.setStartPos(newPos.toPoint()); });
        control->removed.listen(this, &Connection::remove);
    });
    _controlB.then([this](Control *control) {
        control->worldPosChanged.listen(&_wire, [this](QPointF newPos) { _wire.setEndPos(newPos.toPoint()); });
        control->removed.listen(this, &Connection::remove);
    });
}

std::unique_ptr<Connection> Connection::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    AxiomModel::ModelRoot *root) {
    QUuid controlA; stream >> controlA;
    QUuid controlB; stream >> controlB;

    return std::make_unique<Connection>(uuid, parentUuid, controlA, controlB, root);
}

void Connection::serialize(QDataStream &stream) const {
    stream << _controlAUuid;
    stream << _controlBUuid;
}
