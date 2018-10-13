#pragma once

#include <QtCore/QSet>
#include <QtCore/QUuid>

#include "../util.h"
#include "common/NamedLambda.h"
#include "common/WatchSequenceOperators.h"

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
    std::optional<OutputItem> findMaybe(InputCollection &collection, const QUuid &uuid) {
        return findMaybe<OutputItem>(collection.begin(), collection.end(), uuid);
    };

    template<class OutputItem, class InputCollection>
    std::optional<OutputItem> findMaybe(const InputCollection &collection, const QUuid &uuid) {
        return findMaybe<OutputItem>(collection.begin(), collection.end(), uuid);
    };

    template<class InputCollection>
    std::optional<typename InputCollection::value_type> findMaybe(InputCollection &collection, const QUuid &uuid) {
        return findMaybe<typename InputCollection::value_type>(collection, uuid);
    }

    template<class InputCollection>
    std::optional<typename InputCollection::value_type> findMaybe(const InputCollection &collection,
                                                                  const QUuid &uuid) {
        return findMaybe<typename InputCollection::value_type>(collection, uuid);
    }

    template<class OutputItem, class InputIterator>
    OutputItem find(InputIterator begin, InputIterator end, const QUuid &uuid) {
        auto result = findMaybe<OutputItem>(std::move(begin), std::move(end), uuid);
        assert(result.has_value());
        return std::move(*result);
    };

    template<class OutputItem, class InputCollection>
    OutputItem find(InputCollection &collection, const QUuid &uuid) {
        return find<OutputItem>(collection.begin(), collection.end(), uuid);
    };

    template<class OutputItem, class InputCollection>
    OutputItem find(const InputCollection &collection, const QUuid &uuid) {
        return find<OutputItem>(collection.begin(), collection.end(), uuid);
    }

    template<class InputCollection>
    typename InputCollection::value_type find(InputCollection &collection, const QUuid &uuid) {
        return find<typename InputCollection::value_type>(collection, uuid);
    };

    template<class InputCollection>
    typename InputCollection::value_type find(const InputCollection &collection, const QUuid &uuid) {
        return find<typename InputCollection::value_type>(collection, uuid);
    }

    template<class FindSequence>
    struct FindMapLambdaData {
        FindSequence &findSequence;
    };

    template<class FindSequence>
    using FindMapLambda =
        AxiomCommon::MapLambda<typename FindSequence::value_type, QUuid, FindMapLambdaData<FindSequence>>;

    template<class UuidSequence, class FindSequence>
    using FindMapGenerator = AxiomCommon::MapGenerator<UuidSequence, FindMapLambda<FindSequence>>;

    template<class UuidSequence, class FindSequence>
    using FindMapSequence = AxiomCommon::Sequence<FindMapGenerator<UuidSequence, FindSequence>>;

    template<class UuidSequence, class FindSequence>
    FindMapSequence<UuidSequence, FindSequence> findMap(UuidSequence uuids, FindSequence &src) {
        FindMapLambdaData<FindSequence> lambdaData = {src};
        return AxiomCommon::map(
            std::move(uuids),
            FindMapLambda<FindSequence>(lambdaData, [](FindMapLambdaData<FindSequence> &data, QUuid uuid) {
                return find(data.findSequence, uuid);
            }));
    }

    template<class ValueType>
    std::vector<ValueType> heapSort(std::vector<ValueType> collection) {
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
        std::vector<ValueType> result;
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

    template<class UuidSequence>
    struct FindAllLambdaData {
        const UuidSequence &sequence;
    };

    template<class FindSequence, class UuidSequence>
    using FindAllLambda =
        AxiomCommon::FilterLambda<typename FindSequence::value_type &, FindAllLambdaData<UuidSequence>>;

    template<class FindSequence, class UuidSequence>
    using FindAllGenerator = AxiomCommon::FilterGenerator<FindSequence, FindAllLambda<FindSequence, UuidSequence>>;

    template<class FindSequence, class UuidSequence>
    using FindAllSequence = AxiomCommon::Sequence<FindAllGenerator<FindSequence, UuidSequence>>;

    template<class FindSequence, class UuidSequence>
    FindAllSequence<FindSequence, UuidSequence> findAll(FindSequence items, const UuidSequence &uuids) {
        FindAllLambdaData<UuidSequence> lambdaData = {uuids};
        return AxiomCommon::filter(
            std::move(items),
            FindAllLambda<FindSequence, UuidSequence>(
                lambdaData, [](FindAllLambdaData<UuidSequence> &data, typename FindSequence::value_type &item) {
                    return data.sequence.find(item->uuid()) != data.sequence.end();
                }));
    }

    template<class Sequence>
    std::shared_ptr<AxiomCommon::Promise<typename Sequence::value_type>> findLater(Sequence input, QUuid uuid) {
        return AxiomCommon::getFirst(AxiomCommon::filterWatch(
            std::move(input), [uuid](const typename Sequence::value_type &base) { return base->uuid() == uuid; }));
    }

    struct FindChildrenLambdaData {
        QUuid uuid;
    };

    template<class FindSequence>
    using FindChildrenLambda = AxiomCommon::FilterLambda<typename FindSequence::value_type &, FindChildrenLambdaData>;

    template<class FindSequence>
    using FindChildrenGenerator = AxiomCommon::FilterGenerator<FindSequence, FindChildrenLambda<FindSequence>>;

    template<class FindSequence>
    using FindChildrenSequence = AxiomCommon::Sequence<FindChildrenGenerator<FindSequence>>;

    template<class FindSequence>
    using FindChildrenWatchSequence = AxiomCommon::WatchSequence<FindChildrenSequence<typename FindSequence::Sequence>,
                                                                 typename FindSequence::Events>;

    template<class FindSequence>
    FindChildrenSequence<FindSequence> findChildren(FindSequence items, QUuid parentUuid) {
        FindChildrenLambdaData lambdaData = {parentUuid};
        return AxiomCommon::filter(
            std::move(items), FindChildrenLambda<FindSequence>(lambdaData, [](FindChildrenLambdaData &data,
                                                                              typename FindSequence::value_type &item) {
                return item->parentUuid() == data.uuid;
            }));
    }

    template<class FindSequence>
    FindChildrenWatchSequence<FindSequence> findChildrenWatch(FindSequence items, QUuid parentUuid) {
        return AxiomCommon::WatchSequence(findChildren(std::move(items.sequence()), parentUuid),
                                          std::move(items.events()));
    }

    template<class S>
    class DependentsGenerator {
    public:
        using Sequence = S;
        using Item = typename Sequence::value_type;
        using Input = Item;
        using Manager = AxiomCommon::GeneratorManager<DependentsGenerator>;

        struct Data {
            Sequence sequence;
            QUuid uuid;
            bool includeSelf;
        };

        Data *data;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        QSet<QUuid> visitedIds;

        explicit DependentsGenerator(Data *data)
            : data(data), sequenceBegin(data->sequence.begin()), sequenceEnd(data->sequence.end()) {}

        std::optional<Item> next() {
            // Objects in a pool are always in heap-sorted order - this means that a child is _always_ after a parent
            // in the array. We can take advantage of that here by doing a simple iteration over all items in the input
            // pool, and checking to see if we've seen the items parent before, by keeping a set of all seen UUIDs.
            // If we have seen the item before, it must be a dependent of the base item, so we yield it and add it to
            // the set.

            while (sequenceBegin != sequenceEnd) {
                auto obj = std::move(*sequenceBegin);
                sequenceBegin++;
                if (obj->uuid() == data->uuid || visitedIds.contains(obj->parentUuid())) {
                    visitedIds.insert(obj->uuid());
                    if (obj->uuid() != data->uuid || data->includeSelf) return std::move(obj);
                }
            }
            return std::nullopt;
        }
    };

    template<class InternalSequence>
    using DependentsSequence = AxiomCommon::Sequence<DependentsGenerator<InternalSequence>>;

    template<class InternalSequence>
    DependentsSequence<InternalSequence> findDependents(InternalSequence input, QUuid uuid, bool includeSelf = true) {
        return DependentsSequence<InternalSequence>(
            typename DependentsGenerator<InternalSequence>::Manager({std::move(input), uuid, includeSelf}));
    }

    template<class S>
    class DistinctByUuidGenerator {
    public:
        using Sequence = S;
        using Item = typename Sequence::value_type;
        using Input = Item;
        using Manager = AxiomCommon::GeneratorManager<DistinctByUuidGenerator>;

        struct Data {
            Sequence sequence;
        };

        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        QSet<QUuid> seenIds;

        explicit DistinctByUuidGenerator(Data *data)
            : sequenceBegin(data->sequence.begin()), sequenceEnd(data->sequence.end()) {}

        std::optional<Item> next() {
            while (sequenceBegin != sequenceEnd) {
                auto obj = std::move(*sequenceBegin);
                sequenceBegin++;

                if (!seenIds.contains(obj->uuid())) {
                    seenIds.insert(obj->uuid());
                    return std::move(obj);
                }
            }
            return std::nullopt;
        }
    };

    template<class InternalSequence>
    using DistinctByUuidSequence = AxiomCommon::Sequence<DistinctByUuidGenerator<InternalSequence>>;

    template<class InternalSequence>
    DistinctByUuidSequence<InternalSequence> distinctByUuid(InternalSequence input) {
        return DistinctByUuidSequence<InternalSequence>(
            typename DistinctByUuidGenerator<InternalSequence>::Manager({std::move(input)}));
    }
}
