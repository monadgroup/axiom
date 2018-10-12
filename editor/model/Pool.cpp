#include "Pool.h"

#include "PoolOperators.h"

using namespace AxiomModel;

static PoolObject *deref(std::unique_ptr<PoolObject> &obj) {
    return obj.get();
}

Pool::Pool() : _baseSequence({}), _sequence(_baseSequence.map(deref)) {}

Pool::~Pool() {
    destroy();
}

PoolObject *Pool::registerObj(std::unique_ptr<AxiomModel::PoolObject> obj) {
    _baseSequence.sequence.push_back(std::move(obj));
    auto ptr = _baseSequence.sequence.back().get();
    _sequence.itemAdded(ptr);
    return ptr;
}

std::unique_ptr<PoolObject> Pool::removeObj(AxiomModel::PoolObject *obj) {
    // find and remove the object from the owned pool
    auto ownedIndex = AxiomUtil::findUnique(_baseSequence.sequence.begin(), _baseSequence.sequence.end(), obj);
    assert(ownedIndex != _baseSequence.sequence.end());

    // move the object out of the array
    auto ownedObj = std::move(*ownedIndex);
    _baseSequence.sequence.erase(ownedIndex);

    // trigger itemRemoved after removing from the pool, so it can't be iterated over
    _sequence.itemRemoved(ownedObj.get());

    return ownedObj;
}

void Pool::destroy() {
    // objects are always sorted as a heap, so we're guaranteed to never remove an object before its parent here
    while (!_baseSequence.sequence.empty()) {
        _baseSequence.sequence.front()->remove();
    }
}
