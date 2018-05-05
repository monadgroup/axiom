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
    class Pool : public BasePoolView {
    public:
        Pool();

        void registerObj(PoolObject *obj);

        void removeObj(PoolObject *obj);

    private:
        std::set<PoolObject*> _objects;
    };

}
