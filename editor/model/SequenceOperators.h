#pragma once

#include "SequenceMapFilter.h"

namespace AxiomModel {

    template<class Collection>
    Sequence<typename Collection::value_type> wrap(const Collection &collection) {
        return Sequence(std::function([&collection]() -> std::function<std::optional<typename Collection::value_type>()> {
            auto begin = collection.begin();
            auto end = collection.end();
            return [begin, end]() mutable -> std::optional<typename Collection::value_type> {
                if (begin == end) return std::optional<typename Collection::value_type>();
                auto result = *begin;
                begin++;
                return std::move(result);
            };
        }));
    };

    template<class OutputItem, class InputCollection>
    SequenceMapFilter<OutputItem, typename InputCollection::value_type> mapFilter(InputCollection collection, std::function<std::optional<OutputItem>(const typename InputCollection::value_type &)> next) {
        return SequenceMapFilter<OutputItem, typename InputCollection::value_type>(std::move(collection), std::move(next));
    };

    template<class OutputItem, class InputCollection>
    SequenceMapFilter<OutputItem, typename InputCollection::value_type> map(InputCollection collection, std::function<OutputItem(const typename InputCollection::value_type &)> next) {
        return mapFilter(std::move(collection), std::function([next](const typename InputCollection::value_type &base) -> std::optional<OutputItem> {
            return next(base);
        }));
    };

    template<class Collection>
    SequenceMapFilter<typename Collection::value_type, typename Collection::value_type> filter(Collection collection, std::function<bool(const typename Collection::value_type &)> next) {
        return mapFilter(std::move(collection), std::function([next](const typename Collection::value_type &base) -> std::optional<typename Collection::value_type> {
            return next(base) ? base : std::optional<typename Collection::value_type>();
        }));
    };

    template<class OutputItem, class InputCollection>
    SequenceMapFilter<OutputItem, typename InputCollection::value_type> dynamicCast(InputCollection collection) {
        return mapFilter(std::move(collection), std::function([](const typename InputCollection::value_type &base) -> std::optional<OutputItem> {
            auto convert = dynamic_cast<OutputItem>(base);
            return convert ? convert : std::optional<OutputItem>();
        }));
    };

    template<class OutputItem, class InputCollection>
    SequenceMapFilter<OutputItem, typename InputCollection::value_type> staticCast(InputCollection collection) {
        return map(std::move(collection), std::function([](const typename InputCollection::value_type &base) {
            return static_cast<OutputItem>(base);
        }));
    };

    template<class OutputItem, class InputCollection>
    SequenceMapFilter<OutputItem, typename InputCollection::value_type> reinterpretCast(InputCollection collection) {
        return map(std::move(collection), std::function([](const typename InputCollection::value_type &base) {
            return reinterpret_cast<OutputItem>(base);
        }));
    };

};
