#pragma once

#include <optional>
#include <vector>

namespace AxiomCommon {

    template<class Item>
    class SlotMap {
    private:
        std::vector<std::optional<Item>> values;
        size_t _size = 0;
        size_t _nextIndex = 0;

        size_t findNextFree(size_t fromIndex) {
            while (fromIndex < values.size() && values[fromIndex]) {
                fromIndex++;
            }
            return fromIndex;
        }

        size_t findNextUsed(size_t fromIndex) {
            while (fromIndex < values.size() && !values[fromIndex]) {
                fromIndex++;
            }
            return fromIndex;
        }

    public:
        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using key = size_t;

        class iterator {
        public:
            using self_type = iterator;
            using iterator_category = std::forward_iterator_tag;

            struct iterator_reference {
                size_t key;
                reference value;
            };

            iterator(SlotMap *map, size_t index) : map(map), index(index) {}

            self_type operator++() {
                self_type i = *this;
                increment();
                return i;
            }

            const self_type operator++(int junk) {
                increment();
                return *this;
            }

            bool operator==(const iterator &rhs) const { return index == rhs.index; }

            bool operator!=(const iterator &rhs) const { return !(*this == rhs); }

            iterator_reference operator*() { return {index, *map->values[index]}; }

            pointer operator->() { return &*map->values[index]; }

        private:
            SlotMap *map;
            key index;

            void increment() { index = map->findNextUsed(index + 1); }
        };

        iterator begin() { return iterator(this, findNextUsed(0)); }

        iterator end() { return iterator(this, values.size()); }

        // since we pop values removed at the end of the map, the internal vector will be empty if we're empty
        bool empty() const { return values.empty(); }

        size_t size() const { return _size; }

        key insert(Item item) {
            auto insertPosition = _nextIndex;
            insertAt(insertPosition, std::move(item));
            return insertPosition;
        }

        template<class Func>
        key insertWith(Func func) {
            auto insertPosition = _nextIndex;
            auto insertValue = func(insertPosition);
            insertAt(insertPosition, std::move(insertValue));
            return insertPosition;
        }

        bool erase(key key) {
            // if the key does not exist, early exit
            if (key >= values.size() || !values[key]) {
                return false;
            }

            // decrement our size and update next index
            _size--;
            if (key < _nextIndex) {
                _nextIndex = key;
            }

            values[key] = std::nullopt;

            // keep the array trimmed by popping empty items from the end
            while (!values.empty() && !values.back()) {
                values.pop_back();
            }

            return true;
        }

        void clear() {
            values.clear();
            _nextIndex = 0;
            _size = 0;
        }

        iterator find(key key) {
            if (key >= values.size() || !values[key]) {
                return end();
            } else {
                return iterator(this, key);
            }
        }

        reference operator[](key key) { return *values[key]; }

    private:
        void insertAt(size_t position, Item value) {
            // insert the item or increase the size of the vector
            if (values.size() <= position) {
                values.emplace_back(std::move(value));
            } else {
                values[position] = std::move(value);
            }

            // increment size since we've added something
            _size++;

            // increment nextIndex to the next available position
            _nextIndex = findNextFree(position + 1);
        }
    };
}
