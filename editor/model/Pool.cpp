#include "Pool.h"

#include "PoolObject.h"

using namespace AxiomModel;

Pool::Pool() : PoolView(_objects, [](const collection_value_type &base) -> std::optional<value_type> { return base; }) {}

void Pool::registerObj(AxiomModel::PoolObject *obj) {
    _objects.emplace(obj);
}

void Pool::removeObj(AxiomModel::PoolObject *obj) {
    _objects.erase(obj);
}
