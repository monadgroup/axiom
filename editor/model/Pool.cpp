#include "Pool.h"

#include "PoolOperators.h"

using namespace AxiomModel;

static PoolObject *deref(std::unique_ptr<PoolObject> *obj) {
    return obj->get();
}

Pool::Pool()
    : _sequence(BaseSequence(indexSequence(AxiomCommon::map(AxiomCommon::iter(&_objects), deref), &index),
                             AxiomCommon::BaseWatchEvents<PoolObject *>())) {}

Pool::~Pool() {
    destroy();
}

PoolObject *Pool::registerObj(std::unique_ptr<AxiomModel::PoolObject> obj) {
    _objects.push_back(std::move(obj));
    auto ptr = _objects.back().get();
    index.insert(ptr->uuid(), ptr);
    _sequence.events().itemAdded()(ptr);
    return ptr;
}

std::unique_ptr<PoolObject> Pool::removeObj(AxiomModel::PoolObject *obj) {
    // find and remove the object from the owned pool
    auto ownedIndex = AxiomUtil::findUnique(_objects.begin(), _objects.end(), obj);
    assert(ownedIndex != _objects.end());

    // move the object out of the array
    auto ownedObj = std::move(*ownedIndex);
    _objects.erase(ownedIndex);
    index.remove(ownedObj->uuid());

    // trigger itemRemoved after removing from the pool, so it can't be iterated over
    _sequence.events().itemRemoved()(ownedObj.get());

    return ownedObj;
}

void Pool::destroy() {
    // objects are always sorted as a heap, so we're guaranteed to never remove an object before its parent here
    while (!_objects.empty()) {
        _objects.front()->remove();
    }
}
