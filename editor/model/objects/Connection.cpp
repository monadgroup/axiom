#include "Connection.h"

#include "Control.h"
#include "NodeSurface.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "compiler/runtime/Control.h"

using namespace AxiomModel;

Connection::Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA, const QUuid &controlB,
                       AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONNECTION, uuid, parentUuid, root),
      _surface(find(root->nodeSurfaces(), parentUuid)), _controlA(find(root->controls(), controlA)),
      _controlB(find(root->controls(), controlB)), _wire(&_surface->grid(), _controlA->wireType(),
                                                         _controlA->worldPos(), _controlB->worldPos()) {
    _controlA->worldPosChanged.connect(&_wire, std::function([this](QPointF newPos) { _wire.setStartPos(newPos); }));
    _controlB->worldPosChanged.connect(&_wire, std::function([this](QPointF newPos) { _wire.setEndPos(newPos); }));
    _controlA->isActiveChanged.connect(&_wire,
                                       std::function([this](bool isActive) { _wire.setStartActive(isActive); }));
    _controlB->isActiveChanged.connect(&_wire, std::function([this](bool isActive) { _wire.setEndActive(isActive); }));
    _wire.activeChanged.connect(_controlA, &Control::setIsActive);
    _wire.activeChanged.connect(_controlB, &Control::setIsActive);
    _controlA->runtimeAttached.connect(this, &Connection::attachRuntime);
    _controlB->runtimeAttached.connect(this, &Connection::attachRuntime);
    _controlA->runtimeAboutToDetach.connect(this, &Connection::detachRuntime);
    _controlB->runtimeAboutToDetach.connect(this, &Connection::detachRuntime);
    _controlA->removed.connect(this, &Connection::remove);
    _controlB->removed.connect(this, &Connection::remove);

    attachRuntime();
}

std::unique_ptr<Connection> Connection::create(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA,
                                               const QUuid &controlB, AxiomModel::ModelRoot *root) {
    return std::make_unique<Connection>(uuid, parentUuid, controlA, controlB, root);
}

std::unique_ptr<Connection> Connection::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                                    AxiomModel::ModelRoot *root) {
    QUuid controlA;
    stream >> controlA;
    QUuid controlB;
    stream >> controlB;

    return create(uuid, parentUuid, controlA, controlB, root);
}

void Connection::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);
    stream << _controlA->uuid();
    stream << _controlB->uuid();
}

void Connection::attachRuntime() {
    std::cout << "Attempting to attach runtime" << std::endl;
    if (!_controlA->runtime() || !_controlB->runtime()) return;

    std::cout << "Attaching runtime" << std::endl;
    (*_controlA->runtime())->connectTo(*_controlB->runtime());
}

void Connection::detachRuntime() {
    std::cout << "Attempting to detach runtime" << std::endl;
    if (!_controlA->runtime() || !_controlB->runtime()) return;

    std::cout << "Detaching runtime" << std::endl;
    (*_controlA->runtime())->disconnectFrom(*_controlB->runtime());
}

void Connection::remove() {
    detachRuntime();
    _wire.remove();
    ModelObject::remove();
}
