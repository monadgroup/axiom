#include "Control.h"
#include <utility>
#include "ControlSurface.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../../util.h"

using namespace AxiomModel;

Control::Control(ControlType controlType, ValueType valueType, const QUuid &uuid, const QUuid &parentUuid, QPoint pos,
                 QSize size, bool selected, QString name, AxiomModel::ModelRoot *root)
    : GridItem(&find(root->controlSurfaces(), parentUuid)->grid(), pos, size, selected),
      ModelObject(ModelType::CONTROL, uuid, parentUuid, root), _controlType(controlType), _valueType(valueType),
      _name(std::move(name)) {
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

void Control::serialize(QDataStream &stream) const {
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

void Control::remove() {
    ModelObject::remove();
}
