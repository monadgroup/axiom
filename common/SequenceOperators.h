#pragma once

#include "Cast.h"
#include "Sequence.h"

namespace AxiomCommon {
    template<class Output>
    class BlankGenerator {
    public:
        using Item = Output;
        using Sequence = void;
        using Manager = GeneratorManager<BlankGenerator>;

        struct Data {};

        explicit BlankGenerator(Data *) {}

        std::optional<Item> next() { return std::nullopt; }
    };

    template<class Output>
    class OnceGenerator {
    public:
        using Item = Output;
        using Data = Item;
        using Manager = GeneratorManager<OnceGenerator>;

        std::optional<Item> item;

        explicit OnceGenerator(Item *item) : item(*item) {}

        std::optional<Item> next() {
            auto val = std::move(item);
            item.reset();
            return val;
        }
    };

    template<class S, class Functor>
    class FilterMapGenerator {
    public:
        using Sequence = S;
        using Input = typename Sequence::value_type;
        using Item = typename std::result_of<Functor(Input)>::type::value_type;
        using Manager = GeneratorManager<FilterMapGenerator>;

        struct Data {
            Sequence sequence;
            Functor functor;
        };

        Data *data;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;

        explicit FilterMapGenerator(Data *data)
            : data(data), sequenceBegin(data->sequence.begin()), sequenceEnd(data->sequence.end()) {}

        static Sequence &sequence(Manager &manager) { return manager.data.sequence; }

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            return manager.data.functor(std::move(input));
        }

        std::optional<Item> next() {
            while (sequenceBegin != sequenceEnd) {
                auto mappedValue = data->functor(std::move(*sequenceBegin));
                sequenceBegin++;

                if (mappedValue) {
                    return mappedValue;
                }
            }

            return std::nullopt;
        }
    };

    template<class S, class Functor>
    class FilterGenerator {
    public:
        using Sequence = S;
        using Item = typename Sequence::value_type;
        using Input = Item;
        using Manager = GeneratorManager<FilterGenerator>;

        struct Data {
            Sequence sequence;
            Functor functor;
        };

        Data *data;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;

        explicit FilterGenerator(Data *data)
            : data(data), sequenceBegin(data->sequence.begin()), sequenceEnd(data->sequence.end()) {}

        static Sequence &sequence(Manager &manager) { return manager.data.sequence; }

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            if (manager.data.functor(input))
                return input;
            else
                return std::nullopt;
        }

        std::optional<Item> next() {
            while (sequenceBegin != sequenceEnd) {
                auto nextValue = std::move(*sequenceBegin);
                sequenceBegin++;

                if (data->functor(nextValue)) {
                    return nextValue;
                }
            }

            return std::nullopt;
        }
    };

    template<class S, class Functor>
    class MapGenerator {
    public:
        using Sequence = S;
        using Input = typename Sequence::value_type;
        using Item = typename std::result_of<Functor(Input)>::type;
        using Manager = GeneratorManager<MapGenerator>;

        struct Data {
            Sequence sequence;
            Functor functor;
        };

        Data *data;
        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;

        explicit MapGenerator(Data *data)
            : data(data), sequenceBegin(data->sequence.begin()), sequenceEnd(data->sequence.end()) {}

        static Sequence &sequence(Manager &manager) { return manager.data.sequence; }

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            return manager.data.functor(std::move(input));
        }

        std::optional<Item> next() {
            if (sequenceBegin != sequenceEnd) {
                auto mappedValue = data->functor(std::move(*sequenceBegin));
                sequenceBegin++;

                return mappedValue;
            }
            return std::nullopt;
        }
    };

    template<class S>
    class FlattenGenerator {
    public:
        using Sequence = S;
        using Item = typename Sequence::value_type::value_type;
        using Manager = GeneratorManager<FlattenGenerator>;

        struct Data {
            Sequence sequence;
        };

        typename Sequence::iterator sequenceBegin;
        typename Sequence::iterator sequenceEnd;
        std::optional<typename Sequence::value_type> innerSequence;
        std::optional<typename Sequence::value_type::iterator> innerBegin;
        std::optional<typename Sequence::value_type::iterator> innerEnd;

        explicit FlattenGenerator(Data *data)
            : sequenceBegin(data->sequence.begin()), sequenceEnd(data->sequence.end()) {}

        std::optional<Item> next() {
            while (!innerBegin || !innerEnd || *innerBegin == *innerEnd) {
                if (sequenceBegin == sequenceEnd) {
                    return std::nullopt;
                }

                innerSequence = std::move(*sequenceBegin);
                innerBegin = innerSequence->begin();
                innerEnd = innerSequence->end();
                sequenceBegin++;
            }

            auto innerValue = std::move(*innerBegin);
            innerBegin++;
            return innerValue;
        }
    };

    template<class Collection>
    class IterGenerator {
    public:
        using Item = typename Collection::value_type *;
        using Manager = GeneratorManager<IterGenerator>;

        struct Data {
            Collection *collection;
        };

        typename Collection::iterator sequenceBegin;
        typename Collection::iterator sequenceEnd;

        explicit IterGenerator(Data *data)
            : sequenceBegin(data->collection->begin()), sequenceEnd(data->collection->end()) {}

        std::optional<Item> next() {
            if (sequenceBegin != sequenceEnd) {
                auto nextValue = &*sequenceBegin;
                sequenceBegin++;

                return nextValue;
            }
            return std::nullopt;
        }
    };

    template<class Collection>
    class IntoIterGenerator {
    public:
        using Item = typename Collection::value_type *;
        using Manager = GeneratorManager<IntoIterGenerator>;

        struct Data {
            Collection collection;
        };

        typename Collection::iterator sequenceBegin;
        typename Collection::iterator sequenceEnd;

        explicit IntoIterGenerator(Data *data)
            : sequenceBegin(data->collection.begin()), sequenceEnd(data->collection.end()) {}

        std::optional<Item *> next() {
            if (sequenceBegin != sequenceEnd) {
                auto nextValue = &*sequenceBegin;
                sequenceBegin++;

                return nextValue;
            }
            return std::nullopt;
        }
    };

    template<class InternalSequence, class FilterMapFunctor>
    using FilterMapSequence = Sequence<FilterMapGenerator<InternalSequence, FilterMapFunctor>>;

    template<class InternalSequence, class FilterFunctor>
    using FilterSequence = Sequence<FilterGenerator<InternalSequence, FilterFunctor>>;

    template<class InternalSequence, class MapFunctor>
    using MapSequence = Sequence<MapGenerator<InternalSequence, MapFunctor>>;

    template<class InternalSequence>
    using FlattenSequence = Sequence<FlattenGenerator<InternalSequence>>;

    template<class Item>
    using BlankSequence = Sequence<BlankGenerator<Item>>;

    template<class Item>
    using OnceSequence = Sequence<OnceGenerator<Item>>;

    template<class Collection>
    using IterSequence = Sequence<IterGenerator<Collection>>;

    template<class Collection>
    using IntoIterSequence = Sequence<IntoIterGenerator<Collection>>;

    template<class Output, class InternalSequence>
    using CastSequence =
        FilterMapSequence<InternalSequence, std::optional<Output> (*)(typename InternalSequence::Item)>;

    // functions to map other sequences
    template<class Sequence, class FilterMapFunctor>
    FilterMapSequence<Sequence, FilterMapFunctor> filterMap(Sequence sequence, FilterMapFunctor functor) {
        return FilterMapSequence<Sequence, FilterMapFunctor>(
            typename FilterMapGenerator<Sequence, FilterMapFunctor>::Manager(
                {std::move(sequence), std::move(functor)}));
    }

    template<class Sequence, class FilterFunctor>
    FilterSequence<Sequence, FilterFunctor> filter(Sequence sequence, FilterFunctor functor) {
        return FilterSequence<Sequence, FilterFunctor>(
            typename FilterGenerator<Sequence, FilterFunctor>::Manager({std::move(sequence), std::move(functor)}));
    }

    template<class Sequence, class MapFunctor>
    MapSequence<Sequence, MapFunctor> map(Sequence sequence, MapFunctor functor) {
        return MapSequence<Sequence, MapFunctor>(
            typename MapGenerator<Sequence, MapFunctor>::Manager({std::move(sequence), std::move(functor)}));
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> dynamicCast(Sequence sequence) {
        return filterMap(std::move(sequence), &AxiomCommon::wrapDynamicCast<Output, typename Sequence::value_type>);
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> staticCast(Sequence sequence) {
        return filterMap(std::move(sequence), &AxiomCommon::wrapStaticCast<Output, typename Sequence::value_type>);
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> reinterpretCast(Sequence sequence) {
        return filterMap(std::move(sequence), &AxiomCommon::wrapReinterpretCast<Output, typename Sequence::value_type>);
    }

    // functions to modify other iterators
    template<class Sequence>
    FlattenSequence<Sequence> flatten(Sequence sequence) {
        return FlattenSequence<Sequence>(typename FlattenGenerator<Sequence>::Manager({std::move(sequence)}));
    }

    // functions to create new iterators
    template<class Item>
    BlankSequence<Item> blank() {
        return BlankSequence<Item>(typename BlankGenerator<Item>::Manager({}));
    }

    template<class Item>
    OnceSequence<Item> once(Item item) {
        return OnceSequence<Item>(typename OnceSequence<Item>::Manager({std::move(item)}));
    }

    template<class Collection>
    IterSequence<Collection> iter(Collection *collection) {
        return IterSequence<Collection>(typename IterGenerator<Collection>::Manager({collection}));
    }

    template<class Collection>
    IntoIterSequence<Collection> intoIter(Collection collection) {
        return IntoIterSequence<Collection>(typename IntoIterGenerator<Collection>::Manager({std::move(collection)}));
    }

    // functions to operate on sequences
    template<class Collection>
    std::vector<typename Collection::value_type> collect(Collection collection) {
        std::vector<typename Collection::value_type> result;
        for (auto &val : collection) {
            result.push_back(std::move(val));
        }
        return result;
    }

    template<class Collection>
    std::optional<typename Collection::value_type> takeAt(Collection collection, size_t index) {
        size_t currentIndex = 0;
        for (auto &val : collection) {
            if (currentIndex == index) return std::move(val);
            currentIndex++;
        }
        return std::nullopt;
    }
}
