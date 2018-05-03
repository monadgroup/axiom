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

    template<class TO, class TI>
    TO find(const CollectionView<TI> &input, const QUuid &uuid) {
        for (const auto &item : input) {
            if (item->uuid() == uuid) {
                return dynamic_cast<TO>(item);
            }
        }
        return nullptr;
    }

    template<class TI>
    TI find(const CollectionView<TI> &input, const QUuid &uuid) {
        return find<TI, TI>(input, uuid);
    }

    template<class TI>
    CollectionView<TI> filterChildren(const CollectionView<TI> &input, QUuid parentUuid) {
        return derive(input, [parentUuid](const TI &base) -> std::optional<TI> {
            return base->parentUuid() == parentUuid ? base : std::optional<TI>();
        });
    }

}
