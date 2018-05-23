#include "CloneReferenceMapper.h"

using namespace AxiomModel;

QUuid CloneReferenceMapper::map(const QUuid &input) {
    auto iter = _values.find(input);
    if (iter != _values.end()) {
        return *iter;
    }

    auto newUuid = QUuid::createUuid();
    _values.insert(input, newUuid);
    return newUuid;
}

void CloneReferenceMapper::set(const QUuid &key, const QUuid &value) {
    _values.insert(key, value);
}
