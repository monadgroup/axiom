#pragma once

#include <QtCore/QSet>
#include <QtCore/QUuid>

#include "WatchSequenceOperators.h"
#include "../util.h"

namespace AxiomModel {

    template<class OutputItem, class InputIterator>
    std::optional<OutputItem> findMaybe(InputIterator begin, InputIterator end, const QUuid &uuid) {
        for (auto i = begin; i != end; i++) {
            auto item = *i;
            if (item->uuid() == uuid) {
                return dynamic_cast<OutputItem>(item);
            }
        }
        return std::optional<OutputItem>();
    };

    template<class OutputItem, class InputCollection>
    std::optional<OutputItem> findMaybe(const InputCollection &collection, const QUuid &uuid) {
        return findMaybe<OutputItem>(collection.begin(), collection.end(), uuid);
    };

    template<class InputCollection>
    std::optional<typename InputCollection::value_type> findMaybe(const InputCollection &collection, const QUuid &uuid) {
        return findMaybe<typename InputCollection::value_type>(collection, uuid);
    }

    template<class OutputItem, class InputIterator>
    OutputItem find(InputIterator begin, InputIterator end, const QUuid &uuid) {
        auto result = findMaybe<OutputItem>(std::move(begin), std::move(end), uuid);
        assert(result.has_value());
        return std::move(*result);
    };

    template<class OutputItem, class InputCollection>
    OutputItem find(const InputCollection &collection, const QUuid &uuid) {
        return find<OutputItem>(collection.begin(), collection.end(), uuid);
    };

    template<class InputCollection>
    typename InputCollection::value_type find(const InputCollection &collection, const QUuid &uuid) {
        return find<typename InputCollection::value_type>(collection, uuid);
    };

    template<class InputCollection>
    std::vector<typename InputCollection::value_type> heapSort(const InputCollection &input) {
        auto collection = collect(input);
        QSet<QUuid> seenIds;
        QSet<QUuid> parentIds;

        // find all top-level items (i.e ones that don't have parents in this collection)
        for (const auto &itm : collection) {
            if (!seenIds.contains(itm->parentUuid())) {
                parentIds.insert(itm->parentUuid());
            }
            parentIds.remove(itm->uuid());
            seenIds.insert(itm->uuid());
        }

        // parentIds is now a list of UUIDs of parents that don't exist in this collection
        std::vector<typename InputCollection::value_type> result;
        while (!collection.empty()) {
            for (auto i = collection.begin(); i < collection.end(); i++) {
                auto &itm = *i;
                if (!parentIds.contains(itm->parentUuid())) continue;

                parentIds.insert(itm->uuid());
                result.push_back(std::move(itm));
                collection.erase(i);
                i--;
            }
        }

        return std::move(result);
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
    Sequence<typename InputCollection::value_type> findDependents(InputCollection input, QUuid uuid, bool includeSelf = true) {
        // Objects in a pool are always in heap-sorted order - this means that a child is _always_ after a parent
        // in the array. We can take advantage of that here by doing a simple iteration over all items in the input
        // pool, and checking to see if we've seen the items parent before, by keeping a set of all seen UUIDs.
        // If we have seen the item before, it must be a dependent of the base item, so we yield it and add it to the
        // set.

        return Sequence<typename InputCollection::value_type>([input, uuid, includeSelf]() {
            std::optional<typename InputCollection::const_iterator> begin;
            std::optional<typename InputCollection::const_iterator> end;
            QSet<QUuid> visitedIds;
            return [uuid, input, begin, end, visitedIds, includeSelf]() mutable -> std::optional<typename InputCollection::value_type> {
                if (!begin || !end) {
                    begin = input.begin();
                    end = input.end();
                }

                while (*begin != *end) {
                    auto obj = **begin;
                    (*begin)++;
                    if (obj->uuid() == uuid || visitedIds.contains(obj->parentUuid())) {
                        visitedIds.insert(obj->uuid());
                        if (obj->uuid() != uuid || includeSelf) return std::move(obj);
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
