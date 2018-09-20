#include "Connection.h"

#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../ReferenceMapper.h"
#include "Control.h"
#include "NodeSurface.h"

using namespace AxiomModel;

Connection::Connection(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlAUuid, const QUuid &controlBUuid,
                       AxiomModel::ModelRoot *root)
    : ModelObject(ModelType::CONNECTION, uuid, parentUuid, root), _surface(find(root->nodeSurfaces(), parentUuid)),
      _controlAUuid(controlAUuid), _controlBUuid(controlBUuid) {
    all(findLater<Control *>(root->controls(), controlAUuid), findLater<Control *>(root->controls(), controlBUuid))
        .then([this](const std::tuple<Control *, Control *> &controls) {
            auto controlA = std::get<0>(controls);
            auto controlB = std::get<1>(controls);
            assert(controlA->wireType() == controlB->wireType());
            _wire.resolve(std::make_unique<ConnectionWire>(&_surface->grid(), &_surface->wireGrid(),
                                                           controlA->wireType(), controlA->worldPos(),
                                                           controlB->worldPos()));
            auto &wire = *_wire.value();
            controlA->worldPosChanged.connect(wire.get(), &ConnectionWire::setStartPos);
            controlB->worldPosChanged.connect(wire.get(), &ConnectionWire::setEndPos);
            controlA->isActiveChanged.connect(wire.get(), &ConnectionWire::setStartActive);
            controlB->isActiveChanged.connect(wire.get(), &ConnectionWire::setEndActive);
            wire->activeChanged.connect(controlA, &Control::setIsActive);
            wire->activeChanged.connect(controlB, &Control::setIsActive);
            controlA->removed.connect(this, &Connection::remove);
            controlB->removed.connect(this, &Connection::remove);
        });
}

std::unique_ptr<Connection> Connection::create(const QUuid &uuid, const QUuid &parentUuid, const QUuid &controlA,
                                               const QUuid &controlB, AxiomModel::ModelRoot *root) {
    return std::make_unique<Connection>(uuid, parentUuid, controlA, controlB, root);
}

void Connection::remove() {
    if (_wire.value()) (*_wire.value())->remove();
    ModelObject::remove();
}
