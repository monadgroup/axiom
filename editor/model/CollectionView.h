#pragma once

#include <optional>
#include <functional>

#include "Event.h"

namespace AxiomModel {

    /**
     * A CollectionView is a "lazy iterator" wrapper of a collection.
     * It can both filter out and map collections, by implementing the `filter` method.
     *
     * Template parameters:
     *  - TI is the output item type
     *  - TC is the collection type
     */
    template<class TI, class TC>
    class CollectionView {
    public:
        using collection_type = TC;

        // sometimes we need to read dependent types on `collection_type`, so remove a possible reference
        using collection_class = typename std::remove_reference<collection_type>::type;
        using collection_iter = typename collection_class::iterator;
        using collection_const_iter = typename collection_class::const_iterator;
        using collection_value_type = typename collection_class::value_type;

        // some types expected for collections
        using value_type = TI;
        using reference = value_type&;
        using const_reference = const reference;
        using pointer = value_type*;
        using const_pointer = const pointer;
        using difference_type = typename collection_class::difference_type;

        using filter_func = typename std::function<std::optional<value_type>(const collection_value_type &base)>;

        // standard iterator
        class iterator {
        public:
            using self_type = iterator;
            using iterator_category = std::forward_iterator_tag;

            iterator(CollectionView *view, collection_iter iter) : _view(view), _iter(iter) {
                increment(false);
            }

            self_type operator++() { self_type i = *this; increment(true); return i; }

            const self_type operator++(int junk) { increment(true); return *this; }

            reference operator*() { return *_last; }

            pointer operator->() { return &*_last; }

            bool operator==(const self_type &rhs) { return _iter == rhs._iter; }

            bool operator!=(const self_type &rhs) { return _iter != rhs._iter; }

        private:
            CollectionView *_view;
            collection_iter _iter;
            std::optional<value_type> _last;

            void increment(bool incr_iter) {
                do {
                    if (incr_iter) _iter++;
                    incr_iter = true;

                    if (_iter == _view->_collection.end()) break;

                    _last = _view->filter(*_iter);
                } while (!_last);
            }
        };

        // standard const iterator
        class const_iterator {
        public:
            using self_type = const_iterator;
            using iterator_category = std::forward_iterator_tag;

            const_iterator(const CollectionView *view, collection_const_iter iter) : _view(view), _iter(iter) {
                increment(false);
            }

            self_type operator++() { self_type i = *this; increment(true); return i; }

            const self_type operator++(int junk) { increment(true); return *this; }

            reference operator*() { return *_last; }

            pointer operator->() { return &*_last; }

            bool operator==(const self_type &rhs) { return _iter == rhs._iter; }

            bool operator!=(const self_type &rhs) { return _iter != rhs._iter; }

        private:
            const CollectionView *_view;
            collection_const_iter _iter;
            std::optional<const value_type> _last;

            void increment(bool incr_iter) {
                do {
                    if (incr_iter) _iter++;
                    incr_iter = true;

                    if (_iter = _view->_collection.end()) break;

                    _last = _view->filter(*_iter);
                } while (!_last);
            }
        };

        Event<const value_type &> itemAdded;
        Event<const value_type &> itemRemoved;

        explicit CollectionView(collection_type collection, filter_func filter) : _collection(collection), _filter(filter) {
            assignFilters(&collection);
        }

        collection_type &collection() { return _collection; }

        const collection_type &collection() const { return _collection; }

        iterator begin() { return iterator(this, _collection.begin()); }

        iterator end() { return iterator(this, _collection.end()); }

        const_iterator begin() const { return const_iterator(this, _collection.begin()); }

        const_iterator end() const { return const_iterator(this, _collection.end()); }

        std::optional<value_type> filter(const collection_value_type &base) {
            return _filter(base);
        }

        std::optional<const value_type> filter(const collection_value_type &base) const {
            return _filter(base);
        }

        std::optional<value_type> filter(const std::optional<collection_value_type> &base) {
            if (!base) return std::optional<value_type>();
            return filter(base);
        }

        std::optional<const value_type> filter(const std::optional<collection_value_type> &base) const {
            if (!base) return std::optional<const value_type>();
            return filter(base);
        }

    private:
        collection_type _collection;
        filter_func _filter;

        void assignFilters(CollectionView *view) {
            view->itemAdded.listen(&CollectionView::addedFilter);
            view->itemRemoved.listen(&CollectionView::removedFilter);
        }

        template<class TG> void assignFilters(const TG &v) {}

        void addedFilter(const collection_value_type &item) const {
            auto filtered = filter(item);
            if (filtered) itemAdded.emit(*filtered);
        }

        void removedFilter(const collection_value_type &item) const {
            auto filtered = filter(item);
            if (filtered) itemRemoved.emit(*filtered);
        }
    };

}
