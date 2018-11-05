#include "Connection.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../PromiseOperators.h"
#include "../ReferenceMapper.h"
#include "Control.h"
#include "NodeSurface.h"

using namespace AxiomModel;

Connection::Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlAUuid, const QUuid &controlBUuid,
                       AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONNECTION, uuid, parentUuid, root),
      _surface(find(root->nodeSurfaces().sequence(), parentUuid)), _controlAUuid(controlAUuid),
      _controlBUuid(controlBUuid) {
    all(findLater(root->controls(), controlAUuid), findLater(root->controls(), controlBUuid))
        ->then([this](const std::tuple<Control *, Control *> &controls) {
            auto controlA = std::get<0>(controls);
            auto controlB = std::get<1>(controls);
            assert(controlA->wireType() == controlB->wireType());
            _wire.resolve(std::make_unique<ConnectionWire>(&_surface->grid(), &_surface->wireGrid(),
                                                           controlA->wireType(), controlA->worldPos(),
                                                           controlB->worldPos()));
            auto &wire = *_wire.value();
            controlA->worldPosChanged.connectTo(wire.get(), &ConnectionWire::setStartPos);
            controlB->worldPosChanged.connectTo(wire.get(), &ConnectionWire::setEndPos);
            controlA->isActiveChanged.connectTo(wire.get(), &ConnectionWire::setStartActive);
            controlB->isActiveChanged.connectTo(wire.get(), &ConnectionWire::setEndActive);
            controlA->isEnabledChanged.connectTo(wire.get(), &ConnectionWire::setStartEnabled);
            controlB->isEnabledChanged.connectTo(wire.get(), &ConnectionWire::setEndEnabled);
            wire->activeChanged.connectTo(controlA, &Control::setIsActive);
            wire->activeChanged.connectTo(controlB, &Control::setIsActive);
            controlA->removed.connectTo(this, &Connection::remove);
            controlB->removed.connectTo(this, &Connection::remove);

            wire->setStartPos(controlA->worldPos());
            wire->setEndPos(controlB->worldPos());
            wire->setStartActive(controlA->isActive());
            wire->setEndActive(controlB->isActive());
            wire->setStartEnabled(controlA->isEnabled());
            wire->setEndEnabled(controlB->isEnabled());
        });
}

std::unique_ptr<Connection> Connection::create(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA,
                                               const QUuid &controlB, AxiomModel::ModelRoot *root) {
    return std::make_unique<Connection>(uuid, parentUuid, controlA, controlB, root);
}

QString Connection::debugName() {
    return "Connection (" + find(root()->controls().sequence(), controlAUuid())->debugName() + " -> " +
           find(root()->controls().sequence(), controlBUuid())->debugName() + ")";
}

void Connection::remove() {
    if (_wire.value()) (*_wire.value())->remove();
    ModelObject::remove();
}
