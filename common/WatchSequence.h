#pragma once

#include "Event.h"
#include "Sequence.h"

namespace AxiomCommon {

    template<class Sequence>
    class BaseWatchSequence {
        using Manager = typename Sequence::Manager;
        using Item = typename Sequence::Item;
        using ItemEvent = Event<Item>;

        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = typename Sequence::iterator;

        Sequence sequence;
        ItemEvent itemAdded;
        ItemEvent itemRemoved;

        explicit BaseWatchSequence(Sequence sequence) : sequence(std::move(sequence)) {}

        iterator begin() const { return sequence.begin(); }

        iterator end() const { return sequence.end(); }

        bool empty() const { return sequence.empty(); }

        size_t size() const { return sequence.size(); }
    };

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
        ItemEvent itemAdded;
        ItemEvent itemRemoved;

        // The provided Sequence should be a FilterMap wrapper around another WatchSequence.
        // The FilterMapped sequence will be exposed as `sequence`, and events from the internal WatchSequence will
        // be passed through the Functor to the events on this WatchSequence.
        explicit WatchSequence(Sequence sequence) : sequence(std::move(sequence)) {
            auto &watchSequence = std::get<Sequence::Generator::SequenceIndex>(sequence.manager.args);
            watchSequence.itemAdded.connect(this, &WatchSequence::parentItemAdded);
            watchSequence.itemRemoved.connect(this, &WatchSequence::parentItemRemoved);
        }

        iterator begin() const { return sequence.begin(); }

        iterator end() const { return sequence.end(); }

        bool empty() const { return sequence.empty(); }

        size_t size() const { return sequence.size(); }

        template<class FilterMapFunctor>
        WatchSequence<FilterMapSequence<WatchSequence, FilterMapFunctor>> filterMap(FilterMapFunctor functor) {
            return WatchSequence(Sequence(GeneratorManager(*this, functor)));
        }

        template<class FilterFunctor>
        WatchSequence<FilterSequence<WatchSequence, FilterFunctor>> filter(FilterFunctor functor) {
            return WatchSequence(Sequence(GeneratorManager(*this, functor)));
        }

        template<class MapFunctor>
        WatchSequence<MapSequence<WatchSequence, MapFunctor>> map(MapFunctor functor) {
            return WatchSequence(Sequence(GeneratorManager(*this, functor)));
        }

        template<class Output>
        WatchSequence<MapSequence<WatchSequence, Output (*)(Item)>> dynamicCast() {
            return map(AxiomCommon::dynamicCast);
        }

        template<class Output>
        WatchSequence<MapSequence<WatchSequence, Output (*)(Item)>> staticCast() {
            return map(AxiomCommon::staticCast);
        }

        template<class Output>
        WatchSequence<MapSequence<WatchSequence, Output (*)(Item)>> reinterpretCast() {
            return map(AxiomCommon::reinterpretCast);
        }

    private:
        void parentItemAdded(InputItem input) {
            auto &functor = std::get<Sequence::Generator::FunctorIndex>(sequence.manager.args);
            auto mappedValue = Sequence::Generator::mapFilter(functor, std::move(input));
            if (mappedValue) {
                itemAdded(std::move(*mappedValue));
            }
        }

        void parentItemRemoved(const InputItem &input) {
            auto &functor = std::get<Sequence::Generator::FunctorIndex>(sequence.manager.args);
            auto mappedValue = Sequence::Generator::mapFilter(functor, std::move(input));
            if (mappedValue) {
                itemRemoved(std::move(*mappedValue));
            }
        }
    };
}
