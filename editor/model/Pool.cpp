#include "Pool.h"

#include "PoolObject.h"

using namespace AxiomModel;

Pool::Pool() : BasePoolView(_objects, [](PoolObject *const &base) -> std::optional<PoolObject*> { return base; }) {}

void Pool::registerObj(AxiomModel::PoolObject *obj) {
    _objects.emplace(obj);
    itemAdded.emit(obj);
}

void Pool::removeObj(AxiomModel::PoolObject *obj) {
    itemRemoved.emit(obj);
    _objects.erase(obj);
}
