#pragma once

#include <QtCore/QSet>
#include <QtCore/QUuid>

#include "WatchSequenceOperators.h"
#include "../util.h"

namespace AxiomModel {

    template<class OutputItem, class InputIterator>
    OutputItem find(InputIterator begin, InputIterator end, const QUuid &uuid) {
        for (auto i = begin; i != end; i++) {
            auto item = *i;
            if (item->uuid() == uuid) {
                return dynamic_cast<OutputItem>(item);
            }
        }
        unreachable;
    };

    template<class OutputItem, class InputCollection>
    OutputItem find(const InputCollection &collection, const QUuid &uuid) {
        return find<OutputItem>(collection.begin(), collection.end(), uuid);
    };

    template<class InputCollection>
    typename InputCollection::value_type find(const InputCollection &collection, const QUuid &uuid) {
        return find<typename InputCollection::value_type>(collection, uuid);
    }

    template<class InputCollection, class IdCollection>
    SequenceMapFilter<typename InputCollection::value_type, typename InputCollection::value_type> findAll(InputCollection collection, IdCollection uuids) {
        return filter(collection, std::function([uuids](const typename InputCollection::value_type &base) -> bool {
            return uuids.find(base->uuid()) != uuids.end();
        }));
    };

    template<class OutputItem, class InputItem>
    AxiomCommon::Promise<OutputItem> findLater(WatchSequence<InputItem> input, QUuid uuid) {
        return getFirst(
            mapFilterWatch(std::move(input), std::function([uuid](const InputItem &base) -> std::optional<OutputItem> {
                if (base->uuid() == uuid) {
                    auto cast = dynamic_cast<OutputItem>(base);
                    return cast ? cast : std::optional<OutputItem>();
                }
                return std::optional<OutputItem>();
            })));
    };

    template<class InputCollection>
    SequenceMapFilter<typename InputCollection::value_type, typename InputCollection::value_type>
    findChildren(InputCollection collection, QUuid parentUuid) {
        return filter(collection, std::function([parentUuid](const typename InputCollection::value_type &base) -> bool {
            return base->parentUuid() == parentUuid;
        }));
    };

    template<class Item>
    WatchSequence<Item> findChildrenWatch(WatchSequence<Item> &input, QUuid parentUuid) {
        return WatchSequence(findChildren(input.sequence(), std::move(parentUuid)), input);
    }

    template<class InputCollection>
    Sequence<typename InputCollection::value_type> findDependents(InputCollection input, QUuid uuid) {
        // Objects in a pool are always in heap-sorted order - this means that a child is _always_ after a parent
        // in the array. We can take advantage of that here by doing a simple iteration over all items in the input
        // pool, and checking to see if we've seen the items parent before, by keeping a set of all seen UUIDs.
        // If we have seen the item before, it must be a dependent of the base item, so we yield it and add it to the
        // set.

        return Sequence<typename InputCollection::value_type>([input, uuid]() {
            auto begin = input.begin();
            auto end = input.end();
            QSet<QUuid> visitedIds;
            return [uuid, begin, end, visitedIds]() mutable -> std::optional<typename InputCollection::value_type> {
                while (begin != end) {
                    auto obj = *begin;
                    begin++;
                    if (obj->uuid() == uuid || visitedIds.contains(obj->parentUuid())) {
                        visitedIds.insert(obj->uuid());
                        return std::move(obj);
                    }
                }
                return std::optional<typename InputCollection::value_type>();
            };
        });
    }

    template<class InputCollection>
    Sequence<typename InputCollection::value_type> distinctByUuid(InputCollection input) {
        return Sequence<typename InputCollection::value_type>([input]() {
            auto begin = input.begin();
            auto end = input.end();
            QSet<QUuid> seenIds;
            return [begin, end, seenIds]() mutable -> std::optional<typename InputCollection::value_type> {
                while (begin != end) {
                    auto obj = *begin;
                    begin++;

                    if (!seenIds.contains(obj->uuid())) {
                        seenIds.insert(obj->uuid());
                        return std::move(obj);
                    }
                }
                return std::optional<typename InputCollection::value_type>();
            };
        });
    }

}
