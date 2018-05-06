#include "Control.h"

#include "ControlSurface.h"
#include "Connection.h"
#include "Node.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../../util.h"

using namespace AxiomModel;

Control::Control(ControlType controlType, ValueType valueType, QUuid uuid, const QUuid &parentUuid, QPoint pos,
                 QSize size, bool selected, QString name, AxiomModel::ModelRoot *root)
    : GridItem(&find(root->controlSurfaces(), parentUuid)->grid(), pos, size, selected),
      ModelObject(ModelType::CONTROL, uuid, parentUuid, root), _surface(find(root->controlSurfaces(), parentUuid)),
      _controlType(controlType), _valueType(valueType), _name(std::move(name)),
      _connections(derive<Connection*, Connection*>(root->connections(), [uuid](Connection *const &connection) -> std::optional<Connection*> {
          if (connection->controlAUuid() == uuid || connection->controlBUuid() == uuid) return connection;
          else return std::optional<Connection*>();
      })), _connectedControls(derive<Control*, Connection*>(_connections, [uuid](Connection *const &connection) -> std::optional<Control*> {
          if (connection->controlAUuid() == uuid) return connection->controlB().value();
          if (connection->controlBUuid() == uuid) return connection->controlA().value();
          return std::optional<Control*>();
      })) {
    posChanged.listen([this](QPoint) { updateSinkPos(); });
    _surface->node()->posChanged.listen([this](QPoint) { updateSinkPos(); });
}

std::unique_ptr<Control> Control::deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid,
                                              AxiomModel::ModelRoot *root) {
    uint8_t controlTypeInt; stream >> controlTypeInt;

    QPoint pos;
    QSize size;
    bool selected;
    GridItem::deserialize(stream, pos, size, selected);

    QString name; stream >> name;

    switch ((ControlType) controlTypeInt) {
        case ControlType::NUM_SCALAR:break;
        case ControlType::MIDI_SCALAR:break;
    }

    unreachable;
}

void Control::serialize(QDataStream &stream, const QUuid &parent, bool withContext) const {
    ModelObject::serialize(stream, parent, withContext);

    stream << (uint8_t) _controlType;
    GridItem::serialize(stream);
    stream << _name;
}

void Control::setName(const QString &name) {
    if (name != _name) {
        _name = name;
        nameChanged.trigger(name);
    }
}

QPointF Control::worldPos() const {
    auto worldPos = pos() + ControlSurface::nodeToControl(_surface->node()->pos());
    auto centerPos = worldPos + QPointF(size().width() / 2., size().height() / 2.);
    return ControlSurface::controlToNode(centerPos);
}

void Control::remove() {
    ModelObject::remove();
}

void Control::updateSinkPos() {
    worldPosChanged.trigger(worldPos());
}
