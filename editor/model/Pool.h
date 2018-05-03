#pragma once

#include <QtCore/QUuid>
#include <unordered_set>

#include "CollectionView.h"

namespace AxiomModel {

    template<class TI, class TP> class TypeFilteredPoolView;

    template<class TP> class ChildFilteredPoolView;

    /**
     * A PoolView is a type of CollectionView specific for PoolObjects or their inheritors
     */
    template<class TI, class TP>
    class PoolView : public CollectionView<TI, TP> {
    public:
        using inherit_type = CollectionView<TI, TP>;
        using typename inherit_type::collection_class;
        using typename inherit_type::collection_type;
        using typename inherit_type::collection_value_type;
        using typename inherit_type::value_type;
        using typename inherit_type::iterator;
        using typename inherit_type::const_iterator;
        using typename inherit_type::filter_func;

        using inherit_type::collection;
        using inherit_type::end;

        explicit PoolView(collection_type collection, filter_func filter) : PoolView::CollectionView(collection, filter) {}

        iterator find(const collection_value_type &k) {
            auto find_iter = collection().find(k);
            if (find_iter == collection().end()) return end();
            return iterator(this, find_iter);
        }

        const_iterator find(const collection_value_type &k) const {
            auto find_iter = collection().find(k);
            if (find_iter == collection().end()) return end();
            return const_iterator(this, find_iter);
        }

        template<class TF = typename std::remove_pointer<value_type>::type>
        TF *find(const QUuid &uuid) {
            for (const auto &item : *this) {
                if (item->uuid() == uuid) {
                    return dynamic_cast<TF*>(item);
                }
            }
            return nullptr;
        }

        template<class TF = typename std::remove_pointer<value_type>::type>
        const TF *find(const QUuid &uuid) const {
            for (const auto &item : *this) {
                if (item->uuid() == uuid) {
                    return dynamic_cast<const TF*>(item);
                }
            }
            return nullptr;
        }

        template<class TF>
        TypeFilteredPoolView<TF, PoolView> filterType() const {
            return TypeFilteredPoolView<TF, PoolView>(*this);
        };

        ChildFilteredPoolView<PoolView> filterChildren(const QUuid &parentUuid) const {
            return ChildFilteredPoolView<PoolView>(*this, parentUuid);
        }
    };

    /**
     * A TypeFilteredPoolView is a PoolView that filters to only iterate items of the provided type
     */
    template<class TI, class TP>
    class TypeFilteredPoolView : public PoolView<TI*, TP> {
    public:
        using inherit_type = PoolView<TI*, TP>;
        using typename inherit_type::collection_type;
        using typename inherit_type::collection_value_type;
        using typename inherit_type::value_type;
        using typename inherit_type::filter_func;

        explicit TypeFilteredPoolView(collection_type collection)
            : TypeFilteredPoolView::PoolView(collection, [](const collection_value_type &base) -> std::optional<value_type> {
                auto convert = dynamic_cast<value_type>(base);
                if (!convert) return std::optional<value_type>();
                else return std::optional<value_type>(convert);
            }) {}
    };

    /**
     * A ChildFilteredPoolView is a PoolView that filters to only iterate items that are children of the provided UUID
     */
    template<class TP>
    class ChildFilteredPoolView : public PoolView<typename TP::value_type, TP> {
    public:
        using inherit_type = PoolView<typename TP::value_type, TP>;
        using typename inherit_type::collection_type;
        using typename inherit_type::collection_value_type;
        using typename inherit_type::value_type;

        explicit ChildFilteredPoolView(collection_type collection, QUuid uuid)
            : ChildFilteredPoolView::PoolView(collection, [uuid](const collection_value_type &base) -> std::optional<value_type> {
                return base->parentUuid() == uuid ? std::optional<value_type>(base) : std::optional<value_type>();
            }) {}
    };

    class PoolObject;

    /**
     * A Pool is the root PoolView that is in charge of maintaining the objects
     */
    class Pool : public PoolView<PoolObject*, std::unordered_set<PoolObject*>&> {
    public:
        Pool();

        void registerObj(PoolObject *obj);

        void removeObj(PoolObject *obj);

    private:
        std::unordered_set<PoolObject*> _objects;
    };

}
