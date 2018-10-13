#pragma once

#include <iterator>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

namespace AxiomCommon {

    template<class Generator>
    class SequenceIterator {
    public:
        using Item = typename Generator::Item;
        using self_type = SequenceIterator;
        using iterator_category = std::forward_iterator_tag;

        SequenceIterator() = default;

        explicit SequenceIterator(Generator generator) : generator(std::move(generator)) { increment(); }

        SequenceIterator(const SequenceIterator &) = default;

        SequenceIterator(SequenceIterator &&) noexcept = default;

        SequenceIterator &operator=(const SequenceIterator &) = default;

        SequenceIterator &operator=(SequenceIterator &&) noexcept = default;

        bool ended() const { return !generator; }

        self_type operator++() {
            self_type i = *this;
            increment();
            return i;
        }

        const self_type operator++(int) {
            increment();
            return *this;
        }

        bool operator==(const SequenceIterator &rhs) const {
            // if either iterator has ended, iterators are only equal if both have ended
            if (ended() || rhs.ended()) {
                return ended() && rhs.ended();
            }

            // otherwise, just compare indexes
            return _index == rhs._index;
        }

        bool operator!=(const SequenceIterator &rhs) const { return !(*this == rhs); }

        Item &operator*() { return *currentValue; }

        Item *operator->() { return &*currentValue; }

    private:
        size_t _index = 0;
        std::optional<Generator> generator;
        std::optional<Item> currentValue;

        void increment() {
            if (generator) {
                currentValue = generator->next();
                _index++;

                if (!currentValue) generator.reset();
            }
        }
    };

    template<class G>
    class GeneratorManager {
    public:
        using Generator = G;
        using Data = typename Generator::Data;

        Data data;

        explicit GeneratorManager(Data data) : data(std::move(data)) {}

        SequenceIterator<Generator> begin() { return SequenceIterator(Generator(&data)); }
    };

    template<class G>
    class Sequence {
    public:
        using Generator = G;
        using Manager = typename Generator::Manager;
        using Item = typename Generator::Item;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = SequenceIterator<Generator>;

        Manager manager;

        explicit Sequence(Manager manager) : manager(std::move(manager)) {}

        iterator begin() { return manager.begin(); }

        iterator end() const { return iterator(); }

        bool empty() { return begin() == end(); }

        size_t size() {
            size_t acc = 0;
            for (auto i = begin(); i != end(); i++) {
                acc++;
            }
            return acc;
        }
    };

    // provides a sequence interface backed up by a reference to a sequence
    template<class S>
    class RefSequence {
    public:
        using Sequence = S;
        using Item = typename Sequence::Item;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = typename Sequence::iterator;

        explicit RefSequence(Sequence *seq) : seq(seq) {}

        iterator begin() { return seq->begin(); }

        iterator end() const { return seq->end(); }

        bool empty() const { return seq->empty(); }

        size_t size() { return seq->size(); }

    private:
        Sequence *seq;
    };

    template<class Item>
    class IteratorAdapter {
    public:
        virtual std::unique_ptr<IteratorAdapter> clone() const = 0;
        virtual bool ended() const = 0;
        virtual void increment() = 0;
        virtual bool equals(IteratorAdapter *adapter) const = 0;
        virtual Item *deref() = 0;
    };

    template<class Item, class IterType>
    class SpecifiedIteratorAdapter : public IteratorAdapter<Item> {
    public:
        IterType iterator;

        explicit SpecifiedIteratorAdapter(IterType iterator) : iterator(std::move(iterator)) {}

        std::unique_ptr<IteratorAdapter<Item>> clone() const override {
            return std::make_unique<SpecifiedIteratorAdapter>(iterator);
        }

        bool ended() const override { return iterator.ended(); }

        void increment() override { ++iterator; }

        bool equals(IteratorAdapter<Item> *adapter) const override {
            // review: does this need to be done safely?
            auto specifiedAdapter = (SpecifiedIteratorAdapter *) adapter;
            return iterator == specifiedAdapter->iterator;
        }

        Item *deref() override { return &*iterator; }
    };

    template<class Item>
    class IteratorAdapterIterator {
    public:
        using self_type = IteratorAdapterIterator;
        using iterator_category = std::forward_iterator_tag;

        explicit IteratorAdapterIterator(std::unique_ptr<IteratorAdapter<Item>> adapter)
            : adapter(std::move(adapter)) {}

        IteratorAdapterIterator(const IteratorAdapterIterator &a) : adapter(a.adapter->clone()) {}

        IteratorAdapterIterator(IteratorAdapterIterator &&) noexcept = default;

        IteratorAdapterIterator &operator=(const IteratorAdapterIterator &a) {
            adapter = a.adapter->clone();
            return *this;
        }

        IteratorAdapterIterator &operator=(IteratorAdapterIterator &&a) noexcept = default;

        self_type operator++() {
            self_type i = *this;
            adapter->increment();
            return i;
        }

        const self_type operator++(int) {
            adapter->increment();
            return *this;
        }

        bool operator==(const IteratorAdapterIterator &rhs) const { return adapter->equals(rhs.adapter.get()); }

        bool operator!=(const IteratorAdapterIterator &rhs) const { return !(*this == rhs); }

        Item &operator*() { return *adapter->deref(); }

        Item *operator->() { return adapter->deref(); }

    private:
        std::unique_ptr<IteratorAdapter<Item>> adapter;
    };

    // boxes a sequence so we don't need to know where the values come from
    template<class I>
    class BoxedSequence {
        class BoxAdapter {
        public:
            virtual std::unique_ptr<BoxAdapter> clone() = 0;
            virtual std::unique_ptr<IteratorAdapter<I>> begin() = 0;
            virtual std::unique_ptr<IteratorAdapter<I>> end() = 0;
            virtual bool empty() = 0;
            virtual size_t size() = 0;
        };

        template<class Sequence>
        class SpecifiedBoxAdapter : public BoxAdapter {
        public:
            Sequence sequence;

            explicit SpecifiedBoxAdapter(Sequence sequence) : sequence(std::move(sequence)) {}

            std::unique_ptr<BoxAdapter> clone() override { return std::make_unique<SpecifiedBoxAdapter>(sequence); }

            std::unique_ptr<IteratorAdapter<I>> begin() override {
                return std::make_unique<SpecifiedIteratorAdapter<I, typename Sequence::iterator>>(sequence.begin());
            }

            std::unique_ptr<IteratorAdapter<I>> end() override {
                return std::make_unique<SpecifiedIteratorAdapter<I, typename Sequence::iterator>>(sequence.end());
            }

            bool empty() override { return sequence.empty(); }

            size_t size() override { return sequence.size(); }
        };

    public:
        using Item = I;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = IteratorAdapterIterator<Item>;

        template<class Sequence>
        explicit BoxedSequence(Sequence sequence)
            : adapter(std::make_unique<SpecifiedBoxAdapter<Sequence>>(std::move(sequence))) {}

        BoxedSequence(BoxedSequence &a) : adapter(a.adapter->clone()) {}

        BoxedSequence(BoxedSequence &&) noexcept = default;

        BoxedSequence &operator=(const BoxedSequence &a) {
            adapter = a.adapter->clone();
            return *this;
        }

        BoxedSequence &operator=(BoxedSequence &&) noexcept = default;

        iterator begin() { return IteratorAdapterIterator(adapter->begin()); }

        iterator end() { return IteratorAdapterIterator(adapter->end()); }

        bool empty() const { return adapter->empty(); }

        size_t size() const { return adapter->size(); }

    private:
        std::unique_ptr<BoxAdapter> adapter;
    };

    template<class Sequence>
    static RefSequence<Sequence> refSequence(Sequence *sequence) {
        return RefSequence<Sequence>(sequence);
    }

    template<class Sequence>
    static BoxedSequence<typename Sequence::value_type> boxSequence(Sequence sequence) {
        return BoxedSequence<typename Sequence::value_type>(std::move(sequence));
    }
}
