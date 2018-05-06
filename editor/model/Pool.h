#pragma once

#include <QtCore/QUuid>
#include <unordered_set>

#include "CollectionView.h"

namespace AxiomModel {

    class PoolObject;

    using BasePoolView = CollectionView<PoolObject*>;

    /**
     * A Pool is the root PoolView that is in charge of maintaining the objects
     */
    class Pool : public CollectionView<PoolObject*> {
    public:
        Pool();

        PoolObject *registerObj(std::unique_ptr<PoolObject> obj);

        void registerObj(PoolObject *obj);

        void removeObj(PoolObject *obj);

    private:
        std::vector<std::unique_ptr<PoolObject>> _ownedObjects;
        std::vector<PoolObject*> _objects;
    };

}
