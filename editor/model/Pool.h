#pragma once

#include <QtCore/QUuid>
#include <unordered_set>
#include <memory>

#include "PoolObject.h"
#include "WatchSequence.h"

namespace AxiomModel {

    class PoolObject;

    class Pool {
    public:
        Pool();

        PoolObject *registerObj(std::unique_ptr<PoolObject> obj);

        void registerObj(PoolObject *obj);

        void removeObj(PoolObject *obj);

        WatchSequence<PoolObject*> &sequence() { return _sequence; }

        const WatchSequence<PoolObject*> &sequence() const { return _sequence; }

    private:
        std::vector<std::unique_ptr<PoolObject>> _ownedObjects;
        std::vector<PoolObject*> _objects;
        WatchSequence<PoolObject*> _sequence;
    };

}
