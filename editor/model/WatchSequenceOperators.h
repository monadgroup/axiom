#pragma once

#include <iostream>

#include "WatchSequence.h"
#include "SequenceOperators.h"
#include "common/Promise.h"

namespace AxiomModel {

    template<class OutputItem, class InputItem>
    WatchSequence<OutputItem>
    mapFilterWatch(WatchSequence<InputItem> input, std::function<std::optional<OutputItem>(const InputItem &)> next) {
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
    AxiomCommon::Promise<Item> getFirst(WatchSequence<Item> input) {
        if (!input.empty()) {
            return AxiomCommon::Promise<Item>::from(*input.begin());
        }

        AxiomCommon::Promise<Item> result;
        input.itemAdded.connect([result](const Item &item) mutable {
            result.resolve(item);
        });

        return std::move(result);
    };

    template<class Item>
    AxiomCommon::Promise<Item> takeAtLater(WatchSequence<Item> input, size_t index) {
        auto inputSize = input.size();
        if (inputSize > index) {
            return AxiomCommon::Promise<Item>::from(takeAt(input, index));
        }

        AxiomCommon::Promise<Item> result;
        input.itemAdded.connect([inputSize, index, result](const Item &item) mutable {
            inputSize++;
            if (inputSize == index + 1) result.resolve(item);
        });

        return std::move(result);
    }

}
