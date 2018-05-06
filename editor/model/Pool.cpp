#include "Pool.h"

#include "PoolObject.h"
#include "../util.h"

using namespace AxiomModel;

Pool::Pool() : BasePoolView(_objects, [](PoolObject *const &base) -> std::optional<PoolObject*> { return base; }) {}

PoolObject *Pool::registerObj(std::unique_ptr<AxiomModel::PoolObject> obj) {
    _ownedObjects.push_back(std::move(obj));
    auto ptr = _ownedObjects.back().get();
    registerObj(ptr);
    return ptr;
}

void Pool::registerObj(AxiomModel::PoolObject *obj) {
    _objects.push_back(obj);
    itemAdded.trigger(obj);
}

void Pool::removeObj(AxiomModel::PoolObject *obj) {
    auto index = std::find(_objects.begin(), _objects.end(), obj);
    assert(index != _objects.end());
    _objects.erase(index);

    auto ownedIndex = AxiomUtil::findUnique(_ownedObjects.begin(), _ownedObjects.end(), obj);
    if (ownedIndex != _ownedObjects.end()) {
        _ownedObjects.erase(ownedIndex);
    }
}
