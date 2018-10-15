#pragma once

#include "Event.h"
#include "Sequence.h"

namespace AxiomCommon {

    template<class Item>
    class BaseWatchEvents {
    public:
        using OutputItem = Item;
        using ItemEvent = Event<OutputItem>;

        Event<Item> &itemAdded() { return _itemAdded; }

        const Event<Item> &itemAdded() const { return _itemAdded; }

        Event<Item> &itemRemoved() { return _itemRemoved; }

        const Event<Item> &itemRemoved() const { return _itemRemoved; }

    private:
        Event<Item> _itemAdded;
        Event<Item> _itemRemoved;
    };

    template<class Sequence, class InputEvents>
    class WatchEvents : public TrackedObject {
    public:
        using InputItem = typename InputEvents::OutputItem;
        using OutputItem = typename Sequence::value_type;
        using ItemEvent = Event<OutputItem>;

        WatchEvents(Sequence sequence, InputEvents inputEvents)
            : _sequence(std::move(sequence)), _inputEvents(std::move(inputEvents)) {
            _inputEvents.itemAdded().connect(this, &WatchEvents::parentItemAdded);
            _inputEvents.itemRemoved().connect(this, &WatchEvents::parentItemRemoved);
        }

        Event<OutputItem> &itemAdded() { return _itemAdded; }

        const Event<OutputItem> &itemAdded() const { return _itemAdded; }

        Event<OutputItem> &itemRemoved() { return _itemRemoved; }

        const Event<OutputItem> &itemRemoved() const { return _itemRemoved; }

    private:
        Sequence _sequence;
        InputEvents _inputEvents;
        Event<OutputItem> _itemAdded;
        Event<OutputItem> _itemRemoved;

        void parentItemAdded(InputItem input) {
            auto mappedValue = Sequence::Generator::mapFilter(_sequence.manager, std::move(input));
            if (mappedValue) {
                _itemAdded(std::move(*mappedValue));
            }
        }

        void parentItemRemoved(InputItem input) {
            auto mappedValue = Sequence::Generator::mapFilter(_sequence.manager, std::move(input));
            if (mappedValue) {
                _itemRemoved(std::move(*mappedValue));
            }
        }
    };

    template<class W>
    class RefWatchEvents {
    public:
        using WatchEvents = W;
        using OutputItem = typename WatchEvents::OutputItem;
        using ItemEvent = Event<OutputItem>;

        explicit RefWatchEvents(WatchEvents *watchEvents) : watchEvents(watchEvents) {}

        Event<OutputItem> &itemAdded() { return watchEvents->itemAdded(); }

        const Event<OutputItem> &itemAdded() const { return watchEvents->itemAdded(); }

        Event<OutputItem> &itemRemoved() { return watchEvents->itemRemoved(); }

        const Event<OutputItem> &itemRemoved() const { return watchEvents->itemRemoved(); }

    private:
        WatchEvents *watchEvents;
    };

    template<class Item>
    class BoxedWatchEvents {
        class BoxAdapter {
        public:
            virtual Event<Item> &itemAdded() = 0;
            virtual Event<Item> &itemRemoved() = 0;
        };

        template<class Events>
        class SpecifiedBoxAdapter : public BoxAdapter {
        public:
            Events events;

            explicit SpecifiedBoxAdapter(Events events) : events(std::move(events)) {}

            Event<Item> &itemAdded() override { return events.itemAdded(); }

            Event<Item> &itemRemoved() override { return events.itemRemoved(); }
        };

    public:
        using OutputItem = Item;

        template<class Events>
        explicit BoxedWatchEvents(Events events)
            : adapter(std::make_unique<SpecifiedBoxAdapter<Events>>(std::move(events))) {}

        Event<OutputItem> &itemAdded() { return adapter->itemAdded(); }

        const Event<OutputItem> &itemAdded() const { return adapter->itemAdded(); }

        Event<OutputItem> &itemRemoved() { return adapter->itemRemoved(); }

        const Event<OutputItem> &itemRemoved() const { return adapter->itemRemoved(); }

    private:
        std::unique_ptr<BoxAdapter> adapter;
    };

    template<class S, class E>
    class BaseWatchSequence {
    public:
        using Sequence = S;
        using Events = E;

        BaseWatchSequence(Sequence sequence, Events events)
            : _sequence(std::move(sequence)), _events(std::move(events)) {}

        Sequence &sequence() { return _sequence; }

        const Sequence &sequence() const { return _sequence; }

        Events &events() { return _events; }

        const Events &events() const { return _events; }

    private:
        Sequence _sequence;
        Events _events;
    };

    template<class S, class InputEvents>
    class WatchSequence {
    public:
        using Sequence = S;
        using Events = WatchEvents<Sequence, InputEvents>;

        WatchSequence(Sequence sequence, InputEvents events)
            : _sequence(std::move(sequence)), _events(_sequence, std::move(events)) {}

        WatchSequence(const WatchSequence &) = delete;

        WatchSequence(WatchSequence &&) noexcept = default;

        WatchSequence &operator=(const WatchSequence &) = delete;

        WatchSequence &operator=(WatchSequence &&) noexcept = default;

        Sequence &sequence() { return _sequence; }

        const Sequence &sequence() const { return _sequence; }

        Events &events() { return _events; }

        const Events &events() const { return _events; }

    private:
        Sequence _sequence;
        Events _events;
    };

    template<class WatchSequence>
    class RefWatchSequence {
    public:
        using Sequence = RefSequence<typename WatchSequence::Sequence>;
        using Events = RefWatchEvents<typename WatchSequence::Events>;

        RefWatchSequence(Sequence sequence, Events events)
            : _sequence(std::move(sequence)), _events(std::move(events)) {}

        Sequence &sequence() { return _sequence; }

        const Sequence &sequence() const { return _sequence; }

        Events &events() { return _events; }

        const Events &events() const { return _events; }

    private:
        Sequence _sequence;
        Events _events;
    };

    template<class Item>
    class BoxedWatchSequence {
    public:
        using Sequence = BoxedSequence<Item>;
        using Events = BoxedWatchEvents<Item>;

        BoxedWatchSequence(Sequence sequence, Events events)
            : _sequence(std::move(sequence)), _events(std::move(events)) {}

        Sequence &sequence() { return _sequence; }

        const Sequence &sequence() const { return _sequence; }

        Events &events() { return _events; }

        const Events &events() const { return _events; }

    private:
        Sequence _sequence;
        Events _events;
    };

    template<class WatchEvents>
    RefWatchEvents<WatchEvents> refWatchEvents(WatchEvents *events) {
        return RefWatchEvents<WatchEvents>(events);
    }

    template<class Sequence>
    RefWatchSequence<Sequence> refWatchSequence(Sequence *sequence) {
        return RefWatchSequence<Sequence>(refSequence(&sequence->sequence()), refWatchEvents(&sequence->events()));
    }

    template<class WatchEvents>
    BoxedWatchEvents<typename WatchEvents::OutputItem> boxWatchEvents(WatchEvents events) {
        return BoxedWatchEvents<typename WatchEvents::OutputItem>(std::move(events));
    }

    template<class Sequence>
    BoxedWatchSequence<typename Sequence::Sequence::value_type> boxWatchSequence(Sequence sequence) {
        return BoxedWatchSequence<typename Sequence::Sequence::value_type>(
            boxSequence(std::move(sequence.sequence())), boxWatchEvents(std::move(sequence.events())));
    }
}
