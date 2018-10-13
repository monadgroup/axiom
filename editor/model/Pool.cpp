#include "Pool.h"

#include "PoolOperators.h"

using namespace AxiomModel;

static PoolObject *deref(std::unique_ptr<PoolObject> *obj) {
    return obj->get();
}

Pool::Pool()
    : _sequence(BaseSequence(AxiomCommon::map(AxiomCommon::iter(&objects), deref),
                             AxiomCommon::BaseWatchEvents<PoolObject *>())) {}

Pool::~Pool() {
    destroy();
}

PoolObject *Pool::registerObj(std::unique_ptr<AxiomModel::PoolObject> obj) {
    objects.push_back(std::move(obj));
    auto ptr = objects.back().get();
    _sequence.events().itemAdded()(ptr);
    return ptr;
}

std::unique_ptr<PoolObject> Pool::removeObj(AxiomModel::PoolObject *obj) {
    // find and remove the object from the owned pool
    auto ownedIndex = AxiomUtil::findUnique(objects.begin(), objects.end(), obj);
    assert(ownedIndex != objects.end());

    // move the object out of the array
    auto ownedObj = std::move(*ownedIndex);
    objects.erase(ownedIndex);

    // trigger itemRemoved after removing from the pool, so it can't be iterated over
    _sequence.events().itemRemoved()(ownedObj.get());

    return ownedObj;
}

void Pool::destroy() {
    // objects are always sorted as a heap, so we're guaranteed to never remove an object before its parent here
    while (!objects.empty()) {
        objects.front()->remove();
    }
}
