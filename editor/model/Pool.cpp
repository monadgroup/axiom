#include "Pool.h"

#include "PoolOperators.h"

using namespace AxiomModel;

Pool::Pool() : _sequence(wrap(_objects)) {}

Pool::~Pool() {
    destroy();
}

PoolObject *Pool::registerObj(std::unique_ptr<AxiomModel::PoolObject> obj) {
    _ownedObjects.push_back(std::move(obj));
    auto ptr = _ownedObjects.back().get();
    registerObj(ptr);
    return ptr;
}

void Pool::registerObj(AxiomModel::PoolObject *obj) {
    _objects.push_back(obj);
    _sequence.itemAdded()(obj);
}

std::unique_ptr<PoolObject> Pool::removeObj(AxiomModel::PoolObject *obj) {
    auto index = std::find(_objects.begin(), _objects.end(), obj);
    assert(index != _objects.end());
    _objects.erase(index);

    // trigger itemRemoved before removing from owner pool, so it's still a valid reference
    _sequence.itemRemoved()(obj);

    auto ownedIndex = AxiomUtil::findUnique(_ownedObjects.begin(), _ownedObjects.end(), obj);
    if (ownedIndex != _ownedObjects.end()) {
        auto ownedObj = std::move(*ownedIndex);
        _ownedObjects.erase(ownedIndex);
        return ownedObj;
    } else {
        return nullptr;
    }
}

void Pool::destroy() {
    // objects are always sorted as a heap, so we're guaranteed to never remove an object before its parent here
    while (!_objects.empty()) {
        _objects.front()->remove();
    }
}
