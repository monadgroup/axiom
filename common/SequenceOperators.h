#pragma once

#include "Cast.h"
#include "Sequence.h"
#include "SingleIter.h"

namespace AxiomCommon {
    template<class Output>
    class BlankGenerator {
    public:
        using Item = Output;
        using Sequence = void;
        using Manager = GeneratorManager<BlankGenerator>;

        struct Data {};

        explicit BlankGenerator(Data *) {}

        static std::optional<Item> find(Manager &manager, const QUuid &id) { return std::nullopt; }

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

        static std::optional<Item> find(Manager &manager, const QUuid &id) { return std::nullopt; }

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
        SingleIter<Input, typename Sequence::iterator> iter;

        explicit FilterMapGenerator(Data *data) : data(data), iter(data->sequence.begin(), data->sequence.end()) {}

        static Sequence &sequence(Manager &manager) { return manager.data.sequence; }

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            return manager.data.functor(std::move(input));
        }

        static std::optional<Item> find(Manager &manager, const QUuid &id) {
            if (auto internalFound = manager.data.sequence.find(id)) {
                return mapFilter(manager, std::move(*internalFound));
            } else
                return std::nullopt;
        }

        std::optional<Item> next() {
            while (auto nextVal = iter.next()) {
                if (auto mappedValue = data->functor(std::move(**nextVal))) {
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
        SingleIter<Item, typename Sequence::iterator> iter;

        explicit FilterGenerator(Data *data) : data(data), iter(data->sequence.begin(), data->sequence.end()) {}

        static Sequence &sequence(Manager &manager) { return manager.data.sequence; }

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            if (manager.data.functor(input))
                return input;
            else
                return std::nullopt;
        }

        static std::optional<Item> find(Manager &manager, const QUuid &id) {
            if (auto internalFound = manager.data.sequence.find(id)) {
                return mapFilter(manager, std::move(*internalFound));
            } else
                return std::nullopt;
        }

        std::optional<Item> next() {
            while (auto nextVal = iter.next()) {
                if (data->functor(**nextVal)) {
                    return std::move(**nextVal);
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
        SingleIter<Input, typename Sequence::iterator> iter;

        explicit MapGenerator(Data *data) : data(data), iter(data->sequence.begin(), data->sequence.end()) {}

        static Sequence &sequence(Manager &manager) { return manager.data.sequence; }

        static std::optional<Item> mapFilter(Manager &manager, Input input) {
            return manager.data.functor(std::move(input));
        }

        static std::optional<Item> find(Manager &manager, const QUuid &id) {
            if (auto internalFound = manager.data.sequence.find(id)) {
                return mapFilter(manager, std::move(*internalFound));
            } else
                return std::nullopt;
        }

        std::optional<Item> next() {
            if (auto nextVal = iter.next()) {
                return data->functor(std::move(**nextVal));
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

        SingleIter<typename Sequence::value_type, typename Sequence::iterator> iter;
        std::optional<typename Sequence::value_type *> innerSequence;
        std::optional<SingleIter<Item, typename Sequence::value_type::iterator>> innerIter;

        explicit FlattenGenerator(Data *data) : iter(data->sequence.begin(), data->sequence.end()) {}

        static std::optional<Item> find(Manager &manager, const QUuid &id) { return std::nullopt; }

        std::optional<Item> next() {
            // update the inner sequence
            std::optional<Item *> nextValue;
            while (!nextValue) {
                if (innerIter) {
                    nextValue = innerIter->next();
                }

                if (!nextValue) {
                    auto nextSequence = iter.next();
                    if (!nextSequence) {
                        innerSequence.reset();
                        innerIter.reset();
                        return std::nullopt;
                    }

                    innerSequence = *nextSequence;
                    innerIter = SingleIter<Item, typename Sequence::value_type::iterator>((*innerSequence)->begin(),
                                                                                          (*innerSequence)->end());
                    nextValue = innerIter->next();
                }
            }

            return std::move(**nextValue);
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

        SingleIter<typename Collection::value_type, typename Collection::iterator> iter;

        explicit IterGenerator(Data *data) : iter(data->collection->begin(), data->collection->end()) {}

        std::optional<Item> next() { return iter.next(); }
    };

    template<class Collection>
    class IntoIterGenerator {
    public:
        using Item = typename Collection::value_type *;
        using Manager = GeneratorManager<IntoIterGenerator>;

        struct Data {
            Collection collection;
        };

        SingleIter<typename Collection::value_type, typename Collection::iterator> iter;

        explicit IntoIterGenerator(Data *data) : iter(data->collection.begin(), data->collection.end()) {}

        std::optional<Item> next() { return iter.next(); }
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
    using DynamicCastSequence = FilterMapSequence<InternalSequence, std::optional<Output> (*)(typename InternalSequence::Item)>;

    template<class Output, class InternalSequence>
    using CastSequence =
        MapSequence<InternalSequence, Output (*)(typename InternalSequence::Item)>;

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
    DynamicCastSequence<Output, Sequence> dynamicCast(Sequence sequence) {
        return filterMap(std::move(sequence), &AxiomCommon::wrapDynamicCast<Output, typename Sequence::value_type>);
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> staticCast(Sequence sequence) {
        return map(std::move(sequence), &AxiomCommon::wrapStaticCast<Output, typename Sequence::value_type>);
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> reinterpretCast(Sequence sequence) {
        return map(std::move(sequence), &AxiomCommon::wrapReinterpretCast<Output, typename Sequence::value_type>);
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
