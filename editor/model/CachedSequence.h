#pragma once

#include <QHash>
#include <QUuid>
#include <vector>

#include "../util.h"
#include "IndexedSequence.h"
#include "common/WatchSequenceOperators.h"

namespace AxiomModel {

    template<class BaseSequence>
    class CachedSequence {
    public:
        using Item = typename BaseSequence::Sequence::value_type;

    private:
        using InternalSequence = std::vector<Item>;
        using InternalEvents = typename BaseSequence::Events;

    public:
        using Sequence = AxiomCommon::RefSequence<IndexedSequence<InternalSequence>>;
        using Events = AxiomCommon::RefWatchEvents<InternalEvents>;

    private:
        struct CachedSequenceData : public AxiomCommon::TrackedObject {
            IndexedSequence<InternalSequence> sequence;
            InternalEvents events;
            QHash<QUuid, Item> index;

            explicit CachedSequenceData(InternalEvents events)
                : sequence(InternalSequence(), &index), events(std::move(events)) {}

            void itemAdded(Item item) {
                sequence.sequence.push_back(std::move(item));
                index.insert(item->uuid(), item);
            }

            void itemRemoved(Item item) {
                index.remove(item->uuid());
                for (auto it = sequence.sequence.begin(); it != sequence.sequence.end(); ++it) {
                    if (*it == item) {
                        sequence.sequence.erase(it);
                        return;
                    }
                }

                // If we got here, the item didn't exist in our collection even though it should.
                // That's bad!
                unreachable;
            }
        };

    public:
        using value_type = Item;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = typename Sequence::iterator;

        explicit CachedSequence(BaseSequence seq)
            : data(std::make_unique<CachedSequenceData>(std::move(seq.events()))) {
            // add all of the items in the sequence into our vector
            for (auto &existingItem : seq.sequence()) {
                data->sequence.sequence.push_back(std::move(existingItem));
            }

            // hook up events to add/remove items in our array
            data->events.itemAdded().connectTo(data.get(), &CachedSequenceData::itemAdded);
            data->events.itemRemoved().connectTo(data.get(), &CachedSequenceData::itemRemoved);
        }

        Sequence sequence() { return AxiomCommon::refSequence(&data->sequence); }

        Events events() { return AxiomCommon::refWatchEvents(&data->events); }

        struct RefType {
            using Sequence = IndexedSequence<InternalSequence>;
            using Events = InternalEvents;
        };

        AxiomCommon::RefWatchSequence<RefType> asRef() {
            return AxiomCommon::RefWatchSequence<RefType>(sequence(), events());
        }

    private:
        std::unique_ptr<CachedSequenceData> data;
    };

    template<class Sequence>
    CachedSequence<Sequence> cacheSequence(Sequence sequence) {
        return CachedSequence<Sequence>(std::move(sequence));
    }
}
