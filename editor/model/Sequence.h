#pragma once

#include <functional>
#include <optional>
#include <memory>

namespace AxiomModel {

    template<class Item>
    class Sequence {
    public:
        using value_type = Item;
        using reference = value_type &;
        using const_reference = const reference;
        using pointer = value_type *;
        using const_pointer = const pointer;

        using next_functor = std::function<std::optional<Item>()>;
        using iter_functor = std::function<next_functor()>;

        class iterator {
        private:
            class SequenceStorage {
            public:
                next_functor next;
                std::optional<Item> currentVal;
                size_t index = 0;

                SequenceStorage(next_functor next) : next(std::move(next)), currentVal(this->next()) {}

                void increment() {
                    currentVal = next();
                    index++;
                }
            };

        public:
            using self_type = iterator;
            using iterator_category = std::forward_iterator_tag;

            explicit iterator(next_functor next) {
                if (auto newImpl = std::make_shared<SequenceStorage>(std::move(next)); newImpl->currentVal) {
                    impl = std::move(newImpl);
                }
            }

            iterator() {}

            bool ended() const {
                return !impl;
            }

            size_t index() const {
                return impl->index;
            }

            self_type operator++() {
                self_type i = *this;
                increment();
                return i;
            }

            const self_type operator++(int junk) {
                increment();
                return *this;
            }

            bool operator==(const iterator &rhs) const {
                // if either iterator has ended, iterators are only equal if both have ended
                if (ended() || rhs.ended()) {
                    return ended() && rhs.ended();
                }

                // otherwise, just compare indexes
                return index() == rhs.index();
            }

            bool operator!=(const iterator &rhs) const {
                return !(*this == rhs);
            }

            reference operator*() {
                return *impl->currentVal;
            }

            pointer operator->() {
                return &*impl->currentVal;
            }

        private:
            std::shared_ptr<SequenceStorage> impl;

            void increment() {
                if (impl) {
                    impl->increment();

                    if (!impl->currentVal) impl.reset();
                }
            }
        };

        using const_iterator = iterator;

        explicit Sequence(iter_functor iter) : _iter(std::move(iter)) {}

        const iter_functor &iter() const { return _iter; }

        const_iterator begin() const {
            return iterator(_iter());
        }

        const_iterator end() const {
            return iterator();
        }

        bool empty() const {
            return begin() == end();
        }

        size_t size() const {
            size_t acc = 0;
            for (auto i = begin(); i != end(); i++) {
                acc++;
            }
            return acc;
        }

    private:
        iter_functor _iter;
    };

}
