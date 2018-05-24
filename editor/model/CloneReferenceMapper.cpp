#include "CloneReferenceMapper.h"

using namespace AxiomModel;

QUuid CloneReferenceMapper::mapUuid(const QUuid &input) {
    if (input.isNull()) return input;

    auto iter = _values.find(input);
    if (iter != _values.end()) {
        return *iter;
    }

    auto newUuid = QUuid::createUuid();
    _values.insert(input, newUuid);
    return newUuid;
}

QPoint CloneReferenceMapper::mapPos(const QUuid &parent, const QPoint &input) {
    auto iter = _pos.find(parent);
    if (iter == _pos.end()) return input;
    else return *iter + input;
}

void CloneReferenceMapper::setUuid(const QUuid &key, const QUuid &value) {
    _values.insert(key, value);
}

void CloneReferenceMapper::setPos(const QUuid &key, const QPoint &value) {
    _pos.insert(key, value);
}
