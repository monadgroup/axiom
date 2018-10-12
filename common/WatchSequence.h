#pragma once

#include "Event.h"
#include "Sequence.h"

namespace AxiomCommon {

    template<class Sequence>
    class WatchSequence : public TrackedObject {
    public:
        using Manager = typename Sequence::Manager;
        using Item = typename Sequence::Item;
        using ItemEvent = Event<Item>;

        using InnerWatchSequence = typename Sequence::Generator::Sequence;
        using InputItem = typename InnerWatchSequence::Item;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = typename Sequence::iterator;

        Sequence sequence;

        // The provided Sequence should be a FilterMap wrapper around another WatchSequence.
        // The FilterMapped sequence will be exposed as `sequence`, and events from the internal WatchSequence will
        // be passed through the Functor to the events on this WatchSequence.
        explicit WatchSequence(Sequence sequence) : sequence(std::move(sequence)) {
            auto &watchSequence = std::get<Sequence::Generator::SequenceIndex>(sequence.manager.args);
            watchSequence.itemAdded().connect(this, &WatchSequence::parentItemAdded);
            watchSequence.itemRemoved().connect(this, &WatchSequence::parentItemRemoved);
        }

        ItemEvent &itemAdded() { return _itemAdded; }

        const ItemEvent &itemAdded() const { return _itemAdded; }

        ItemEvent &itemRemoved() { return _itemRemoved; }

        const ItemEvent &itemRemoved() const { return _itemRemoved; }

        iterator begin() const { return sequence.begin(); }

        iterator end() const { return sequence.end(); }

        bool empty() const { return sequence.empty(); }

        size_t size() const { return sequence.size(); }

    private:
        ItemEvent _itemAdded;
        ItemEvent _itemRemoved;

        void parentItemAdded(InputItem input) {
            auto mappedValue = Sequence::Generator::mapFilter(sequence.manager, std::move(input));
            if (mappedValue) {
                itemAdded(std::move(*mappedValue));
            }
        }

        void parentItemRemoved(const InputItem &input) {
            auto mappedValue = Sequence::Generator::mapFilter(sequence.manager, std::move(input));
            if (mappedValue) {
                itemRemoved(std::move(*mappedValue));
            }
        }
    };

    template<class Sequence>
    class BaseWatchSequence {
    public:
        using Item = typename Sequence::value_type;
        using ItemEvent = Event<Item>;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = typename Sequence::iterator;

        Sequence sequence;

        explicit BaseWatchSequence(Sequence sequence) : sequence(std::move(sequence)) {}

        ItemEvent &itemAdded() { return _itemAdded; }

        const ItemEvent &itemAdded() const { return _itemAdded; }

        ItemEvent &itemRemoved() { return _itemRemoved; }

        const ItemEvent &itemRemoved() const { return _itemRemoved; }

        iterator begin() const { return sequence.begin(); }

        iterator end() const { return sequence.end(); }

        bool empty() const { return sequence.empty(); }

        size_t size() const { return sequence.size(); }

    private:
        ItemEvent _itemAdded;
        ItemEvent _itemRemoved;
    };

    template<class I>
    class BoxedWatchSequence {
        class BoxAdapter {
        public:
            virtual Event<I> &itemAdded() = 0;
            virtual Event<I> &itemRemoved() = 0;
            virtual std::unique_ptr<IteratorAdapter<I>> begin() const = 0;
            virtual std::unique_ptr<IteratorAdapter<I>> end() const = 0;
            virtual bool empty() const = 0;
            virtual size_t size() const = 0;
        };

        template<class Sequence>
        class SpecifiedBoxAdapter : public BoxAdapter {
        public:
            Sequence sequence;

            explicit SpecifiedBoxAdapter(Sequence sequence) : sequence(std::move(sequence)) {}

            Event<I> &itemAdded() override { return sequence.itemAdded(); }

            Event<I> &itemRemoved() override { return sequence.itemRemoved(); }

            std::unique_ptr<IteratorAdapter<I>> begin() const override {
                return std::make_unique<SpecifiedIteratorAdapter>(sequence.begin());
            }

            std::unique_ptr<IteratorAdapter<I>> end() const override {
                return std::make_unique<SpecifiedIteratorAdapter>(sequence.end());
            }

            bool empty() const override { return sequence.empty(); }

            size_t size() const override { return sequence.size(); }
        };

    public:
        using Item = I;
        using ItemEvent = Event<Item>;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = IteratorAdapterIterator<Item>;

        template<class Sequence>
        explicit BoxedWatchSequence(Sequence sequence)
            : adapter(std::make_unique<SpecifiedBoxAdapter<Sequence>>(std::move(sequence))) {}

        ItemEvent &itemAdded() { return adapter->itemAdded(); }

        const ItemEvent &itemAdded() const { return adapter->itemAdded(); }

        ItemEvent &itemRemoved() { return adapter->itemRemoved(); }

        const ItemEvent &itemRemoved() const { return adapter->itemRemoved(); }

        iterator begin() const { return IteratorAdapterIterator(adapter->begin()); }

        iterator end() const { return IteratorAdapterIterator(adapter->end()); }

        bool empty() const { return adapter->empty(); }

        size_t size() const { return adapter->size(); }

    private:
        std::unique_ptr<BoxAdapter> adapter;
    };
}
