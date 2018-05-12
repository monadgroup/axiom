#pragma once

#include "WatchSequenceOperators.h"

namespace AxiomModel {

    template<class OutputItem, class InputIterator>
    OutputItem find(InputIterator begin, InputIterator end, const QUuid &uuid) {
        for (auto i = begin; i != end; i++) {
            auto item = *i;
            if (item->uuid() == uuid) {
                return dynamic_cast<OutputItem>(item);
            }
        }
        return nullptr;
    };

    template<class OutputItem, class InputCollection>
    OutputItem find(const InputCollection &collection, const QUuid &uuid) {
        return find<OutputItem>(collection.begin(), collection.end(), uuid);
    };

    template<class InputCollection>
    typename InputCollection::value_type find(const InputCollection &collection, const QUuid &uuid) {
        return find<typename InputCollection::value_type>(collection, uuid);
    }

    template<class OutputItem, class InputItem>
    Promise<OutputItem> findLater(WatchSequence<InputItem> input, QUuid uuid) {
        return getFirst(mapFilterWatch(std::move(input), std::function([uuid](const InputItem &base) -> std::optional<OutputItem> {
            if (base->uuid() == uuid) {
                auto cast = dynamic_cast<OutputItem>(base);
                return cast ? cast : std::optional<OutputItem>();
            }
            return std::optional<OutputItem>();
        })));
    };

    template<class Item>
    WatchSequence<Item> findChildren(WatchSequence<Item> &input, QUuid parentUuid) {
        return mapFilterWatch(input, std::function([parentUuid](const Item &base) -> std::optional<Item> {
            return base->parentUuid() == parentUuid ? base : std::optional<Item>();
        }));
    }

}
