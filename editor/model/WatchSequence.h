#pragma once

#include <iostream>

#include "SequenceMapFilter.h"
#include "common/Event.h"

namespace AxiomModel {

    template<class Item>
    class WatchSequence {
    public:
        using sequence_type = Sequence<Item>;
        using value_type = typename sequence_type::value_type;
        using reference = typename sequence_type::reference;
        using const_reference = typename sequence_type::const_reference;
        using pointer = typename sequence_type::pointer;
        using const_pointer = typename sequence_type::const_pointer;
        using iterator = typename sequence_type::iterator;
        using const_iterator = typename sequence_type::const_iterator;

        using ItemEvent = AxiomCommon::Event<const Item &>;

        explicit WatchSequence(sequence_type sequence)
            : _sequence(std::move(sequence)), _itemAdded(std::make_shared<EventBinding>()),
              _itemRemoved(std::make_shared<EventBinding>()) {}

        template<class InputItem>
        WatchSequence(const SequenceMapFilter<Item, InputItem> &mapFilter, WatchSequence<InputItem> &parent)
            : _sequence(mapFilter.sequence()), _itemAdded(std::make_shared<EventBinding>(parent.itemAddedBinding())),
              _itemRemoved(std::make_shared<EventBinding>(parent.itemRemovedBinding())) {
            auto mapFilterNext = mapFilter.next();

            // Forward events from the parent sequence.
            // We want these events to continue running even if this class is destroyed (as long as something is
            // receiving the events), hence why we use shared pointers. We don't, however, want to keep these events
            // going if the only reference to them is the code here to forward events to them - hence we use weak
            // pointers. Note that locking the weak pointer inside the callback is always safe, as Event.connect
            // will disconnect when `_itemAdded` is destructed.
            auto itemAddedEvent = _itemAdded.get();
            auto itemRemovedEvent = _itemRemoved.get();
            parent.itemAdded().connect(&itemAddedEvent->event, [itemAddedEvent, mapFilterNext](const InputItem &input) {
                std::cout << "Got itemAdded event, ";
                auto result = mapFilterNext(input);
                if (result) {
                    std::cout << "forwarding it on" << std::endl;
                    itemAddedEvent->event(*result);
                } else {
                    std::cout << "ignoring it" << std::endl;
                }
            });
            parent.itemRemoved().connect(&itemRemovedEvent->event,
                                         [itemRemovedEvent, mapFilterNext](const InputItem &input) {
                                             std::cout << "Got itemRemoved event, ";
                                             auto result = mapFilterNext(input);
                                             if (result) {
                                                 std::cout << "forwarding it on" << std::endl;
                                                 itemRemovedEvent->event(*result);
                                             } else {
                                                 std::cout << "ignoring it" << std::endl;
                                             }
                                         });
        }

        ItemEvent &itemAdded() { return _itemAdded->event; }

        const ItemEvent &itemAdded() const { return _itemAdded->event; }

        ItemEvent &itemRemoved() { return _itemRemoved->event; }

        const ItemEvent &itemRemoved() const { return _itemRemoved->event; }

        std::shared_ptr<void> itemAddedBinding() { return _itemAdded; }

        std::shared_ptr<void> itemRemovedBinding() { return _itemRemoved; }

        sequence_type &sequence() { return _sequence; }

        const sequence_type &sequence() const { return _sequence; }

        const_iterator begin() const { return _sequence.begin(); }

        const_iterator end() const { return _sequence.end(); }

        bool empty() const { return _sequence.empty(); }

        size_t size() const { return _sequence.size(); }

    private:
        sequence_type _sequence;
        struct EventBinding {
            ItemEvent event;
            std::shared_ptr<void> parentEvent;

            EventBinding() {}

            EventBinding(std::shared_ptr<void> parentEvent) : parentEvent(parentEvent) {}
        };

        std::shared_ptr<EventBinding> _itemAdded;
        std::shared_ptr<EventBinding> _itemRemoved;
    };
}
