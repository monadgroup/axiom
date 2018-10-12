#pragma once

#include "Cast.h"
#include "Sequence.h"

namespace AxiomCommon {

    template<class InternalSequence, class FilterMapFunctor>
    using FilterMapSequence = Sequence<FilterMapGenerator<InternalSequence, FilterMapFunctor>>;

    template<class InternalSequence, class FilterFunctor>
    using FilterSequence = Sequence<FilterGenerator<InternalSequence, FilterFunctor>>;

    template<class InternalSequence, class MapFunctor>
    using MapSequence = Sequence<MapGenerator<InternalSequence, MapFunctor>>;

    template<class InternalSequence>
    using FlattenSequence = Sequence<FlattenGenerator<InternalSequence>>;

    template<class Output, class InternalSequence>
    using CastSequence = MapSequence<InternalSequence, Output (*)(typename InternalSequence::Item)>;

    // functions to map other sequences
    template<class Sequence, class FilterMapFunctor>
    FilterMapSequence<Sequence, FilterMapFunctor> filterMap(Sequence sequence, FilterMapFunctor functor) {
        return Sequence(GeneratorManager(std::move(sequence), std::move(functor)));
    }

    template<class Sequence, class FilterFunctor>
    FilterSequence<Sequence, FilterFunctor> filter(Sequence sequence, FilterFunctor functor) {
        return Sequence(GeneratorManager(std::move(sequence), std::move(functor)));
    }

    template<class Sequence, class MapFunctor>
    MapSequence<Sequence, MapFunctor> map(Sequence sequence, MapFunctor functor) {
        return Sequence(GeneratorManager(std::move(sequence), std::move(functor)));
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> dynamicCast(Sequence sequence) {
        return map(std::move(sequence), AxiomCommon::wrapDynamicCast);
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> staticCast(Sequence sequence) {
        return map(std::move(sequence), AxiomCommon::wrapStaticCast);
    }

    template<class Output, class Sequence>
    CastSequence<Output, Sequence> reinterpretCast(Sequence sequence) {
        return map(std::move(sequence), AxiomCommon::wrapReinterpretCast);
    }

    // functions to modify other iterators
    template<class Sequence>
    FlattenSequence<Sequence> flatten(Sequence sequence) {
        return Sequence(GeneratorManager(std::move(sequence)));
    }

    // functions to create new iterators
    template<class Item>
    Sequence<BlankGenerator<Item>> blank() {
        return Sequence(GeneratorManager<BlankGenerator<Item>>());
    }

    template<class Item>
    Sequence<OnceGenerator<Item>> once(Item item) {
        return Sequence(GeneratorManager(std::move(item)));
    }

    template<class Collection>
    Sequence<IterGenerator<Collection>> iter(Collection &collection) {
        return Sequence(GeneratorManager(collection));
    }

    template<class Collection>
    Sequence<IntoIterGenerator<Collection>> intoIter(Collection collection) {
        return Sequence(GeneratorManager(std::forward(collection)));
    }
}
