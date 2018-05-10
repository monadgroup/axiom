#pragma once

#include <optional>
#include <functional>
#include <memory>

#include "Event.h"
#include "Promise.h"

namespace AxiomModel {

    // the user should see an interface like:
    template<class TI>
    class CollectionView {
    public:
        using value_type = TI;
        using reference = value_type&;
        using const_reference = const reference;
        using pointer = value_type*;
        using const_pointer = const pointer;

    private:
        class IteratorImpl {
        public:
            virtual void increment(bool incr_iter) = 0;
            virtual pointer last() = 0;
            virtual std::unique_ptr<IteratorImpl> clone() const = 0;
        };

    public:
        class iterator {
        public:
            using self_type = iterator;
            using iterator_category = std::forward_iterator_tag;

            explicit iterator(std::unique_ptr<IteratorImpl> impl) : impl(std::move(impl)) {}

            iterator(const iterator &a) : impl(a.impl->clone()) {}

            iterator &operator=(const iterator &a) {
                impl = a.impl->clone();
                return *this;
            }

            self_type operator++() { self_type i = *this; impl->increment(true); return i; }

            const self_type operator++(int junk) { impl->increment(true); return *this; }

            reference operator*() { return *impl->last(); }

            pointer operator->() { return impl->last(); }

            bool operator==(const self_type &rhs) { return impl->last() == rhs.impl->last(); }

            bool operator!=(const self_type &rhs) { return impl->last() != rhs.impl->last(); }

        private:
            std::unique_ptr<IteratorImpl> impl;
        };

        using const_iterator = iterator;

    private:

        class AbstractImpl : public Hookable {
        public:
            Event<const value_type &> itemAdded;
            Event<const value_type &> itemRemoved;

            virtual iterator begin() = 0;
            virtual iterator end() = 0;
            virtual const_iterator begin() const = 0;
            virtual const_iterator end() const = 0;
            virtual bool empty() const = 0;
            virtual size_t size() const = 0;
            virtual std::unique_ptr<AbstractImpl> clone() const = 0;
        };

        template<class TC>
        class ConverterImpl : public AbstractImpl {
        private:
            using collection_type = TC;
            using collection_class = typename std::remove_reference<collection_type>::type;
            using collection_iter = typename collection_class::iterator;
            using collection_const_iter = typename collection_class::const_iterator;
            using collection_value_type = typename collection_class::value_type;

            using filter_func = typename std::function<std::optional<value_type>(const collection_value_type &)>;

        private:
            class ConverterIteratorImpl : public IteratorImpl {
            public:
                ConverterIteratorImpl(ConverterImpl *converter, collection_iter iter, std::optional<value_type> last)
                    : _converter(converter), _iter(iter), _last(last) {
                    increment(false);
                }

                void increment(bool incrIter) override {
                    do {
                        if (incrIter) _iter++;
                        incrIter = true;

                        if (_iter == _converter->collection.end()) break;

                        _last = _converter->filter(*_iter);
                    } while (!_last);
                }

                pointer last() override {
                    return &*_last;
                }

                std::unique_ptr<IteratorImpl> clone() const {
                    return std::make_unique<ConverterIteratorImpl>(_converter, _iter, _last);
                }

            private:
                ConverterImpl *_converter;
                collection_iter _iter;
                std::optional<value_type> _last;
            };

            class ConverterConstInteratorImpl : public IteratorImpl {
            public:
                ConverterConstInteratorImpl(const ConverterImpl *converter, collection_const_iter iter, std::optional<const value_type> last)
                    : _converter(converter), _iter(iter), _last(last) {
                    increment(false);
                }

                void increment(bool incrIter) override {
                    do {
                        if (incrIter) _iter++;
                        incrIter = true;

                        if (_iter == _converter->collection.end()) break;

                        _last = std::move(_converter->filter(*_iter));
                    } while (!_last);
                }

                pointer last() override {
                    return &*_last;
                }

                std::unique_ptr<IteratorImpl> clone() const {
                    return std::make_unique<ConverterConstInteratorImpl>(_converter, _iter, _last);
                }

            private:
                const ConverterImpl *_converter;
                collection_const_iter _iter;
                std::optional<value_type> _last;
            };

        public:

            ConverterImpl(collection_type collection, filter_func filter)
                : collection(collection), filter(filter) {
                attachEvent(collection);
            }

            iterator begin() {
                return iterator(std::make_unique<ConverterIteratorImpl>(this, collection.begin(), std::optional<value_type>()));
            }

            iterator end() {
                return iterator(std::make_unique<ConverterIteratorImpl>(this, collection.end(), std::optional<value_type>()));
            }

            const_iterator begin() const {
                return const_iterator(std::make_unique<ConverterConstInteratorImpl>(this, collection.begin(), std::optional<value_type>()));
            }

            const_iterator end() const {
                return const_iterator(std::make_unique<ConverterConstInteratorImpl>(this, collection.end(), std::optional<value_type>()));
            }

            bool empty() const {
                if (collection.empty()) return true;
                return begin() == end();
            }

            size_t size() const {
                size_t acc = 0;
                for (auto i = begin(); i != end(); i++) {
                    acc++;
                }
                return acc;
            }

            std::unique_ptr<AbstractImpl> clone() const {
                return std::make_unique<ConverterImpl>(collection, filter);
            }

        private:
            TC collection;
            filter_func filter;

            void attachEvent(CollectionView<collection_value_type> &collection) {
                collection.itemAdded.connect(this, &ConverterImpl::filterItemAdded);
                collection.itemRemoved.connect(this, &ConverterImpl::filterItemRemoved);
            }

            template<class Dummy> void attachEvent(Dummy &collection) { }

            void filterItemAdded(const collection_value_type &itm) {
                auto result = filter(itm);
                if (result) AbstractImpl::itemAdded.trigger(*result);
            }

            void filterItemRemoved(const collection_value_type &itm) {
                auto result = filter(itm);
                if (result) AbstractImpl::itemRemoved.trigger(*result);
            }
        };

    public:
        Event<const value_type &> itemAdded;
        Event<const value_type &> itemRemoved;

        template<class TC>
        CollectionView(TC collection, std::function<std::optional<TI>(const typename std::remove_reference<TC>::type::value_type &)> func) {
            impl = std::make_unique<ConverterImpl<TC>>(std::move(collection), std::move(func));
            impl->itemAdded.connect(&itemAdded);
            impl->itemRemoved.connect(&itemRemoved);
        }

        template<class TC>
        static CollectionView create(TC collection, std::function<std::optional<TI>(const typename std::remove_reference<TC>::type::value_type &)> func) {
            return CollectionView(collection, func);
        }

        CollectionView(const CollectionView &a) : impl(a.impl->clone()) {}

        CollectionView &operator=(const CollectionView &a) {
            if (impl) {
                impl->itemAdded.disconnect(&itemAdded);
                impl->itemRemoved.disconnect(&itemRemoved);
            }

            impl = a.impl->clone();
            impl->itemAdded.connect(&itemAdded);
            impl->itemRemoved.connect(&itemRemoved);
            return *this;
        }

        iterator begin() { return impl->begin(); }

        iterator end() { return impl->end(); }

        const_iterator begin() const { return impl->begin(); }

        const_iterator end() const { return impl->end(); }

        bool empty() const { return impl->empty(); }

        size_t size() const { return impl->size(); }

    private:
        std::unique_ptr<AbstractImpl> impl;
    };
};
