#pragma once

#include <iostream>
#include "SequenceMapFilter.h"
#include "common/Event.h"

namespace AxiomModel {

    template<class Item>
    class WatchSequence : public AxiomCommon::Hookable {
    public:
        using sequence_type = Sequence<Item>;
        using value_type = typename sequence_type::value_type;
        using reference = typename sequence_type::reference;
        using const_reference = typename sequence_type::const_reference;
        using pointer = typename sequence_type::pointer;
        using const_pointer = typename sequence_type::const_pointer;
        using iterator = typename sequence_type::iterator;
        using const_iterator = typename sequence_type::const_iterator;

        AxiomCommon::Event<const Item &> itemAdded;
        AxiomCommon::Event<const Item &> itemRemoved;

        explicit WatchSequence(sequence_type sequence) : _sequence(std::move(sequence)) {}

        template<class InputItem>
        WatchSequence(const SequenceMapFilter<Item, InputItem> &mapFilter, AxiomCommon::Event<const InputItem &> &parentAdded, AxiomCommon::Event<const InputItem &> &parentRemoved) : _sequence(mapFilter.sequence()) {
            auto mapFilterNext = mapFilter.next();
            auto itemAddedEvent = itemAdded;
            auto itemRemovedEvent = itemRemoved;
            parentAdded.connect(&itemAdded, std::function([itemAddedEvent, mapFilterNext](const InputItem &input) {
                auto result = mapFilterNext(input);
                if (result) itemAddedEvent.trigger(*result);
            }));
            parentRemoved.connect(&itemRemoved, std::function([itemRemovedEvent, mapFilterNext](const InputItem &input) {
                auto result = mapFilterNext(input);
                if (result) itemRemovedEvent.trigger(*result);
            }));
        }

        template<class InputItem>
        WatchSequence(const SequenceMapFilter<Item, InputItem> &mapFilter, WatchSequence<InputItem> &parent)
            : WatchSequence(mapFilter, parent.itemAdded, parent.itemRemoved) {}

        sequence_type &sequence() { return _sequence; }

        const sequence_type &sequence() const { return _sequence; }

        const_iterator begin() const {
            return _sequence.begin();
        }

        const_iterator end() const {
            return _sequence.end();
        }

        bool empty() const {
            return _sequence.empty();
        }

        size_t size() const {
            return _sequence.size();
        }

    private:
        sequence_type _sequence;
    };

}
