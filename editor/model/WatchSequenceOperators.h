#pragma once

#include <iostream>

#include "WatchSequence.h"
#include "SequenceOperators.h"
#include "Promise.h"

namespace AxiomModel {

    template<class OutputItem, class InputItem>
    WatchSequence<OutputItem> mapFilterWatch(WatchSequence<InputItem> input, std::function<std::optional<OutputItem>(const InputItem &)> next) {
        return WatchSequence(mapFilter(input.sequence(), std::move(next)), input);
    };

    template<class Item>
    WatchSequence<Item> filterWatch(WatchSequence<Item> input, std::function<bool(const Item &)> next) {
        return WatchSequence(filter(input.sequence(), std::move(next)), input);
    };

    template<class OutputItem, class InputItem>
    WatchSequence<OutputItem> dynamicCastWatch(WatchSequence<InputItem> input) {
        return WatchSequence(dynamicCast<OutputItem>(input.sequence()), input);
    };

    template<class OutputItem, class InputItem>
    WatchSequence<OutputItem> staticCastWatch(WatchSequence<InputItem> input) {
        return WatchSequence(staticCast<OutputItem>(input.sequence()), input);
    };

    template<class OutputItem, class InputItem>
    WatchSequence<OutputItem> reinterpretCastWatch(WatchSequence<InputItem> input) {
        return WatchSequence(reinterpretCast<OutputItem>(input.sequence()), input);
    };

    template<class Item>
    Promise<Item> getFirst(WatchSequence<Item> input) {
        if (!input.empty()) {
            return Promise<Item>::from(*input.begin());
        }

        Promise<Item> result;
        input.itemAdded.connect([result](const Item &item) mutable {
            result.resolve(item);
        });

        return std::move(result);
    };

}
