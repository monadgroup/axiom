#pragma once

#include "SequenceOperators.h"
#include "WatchSequence.h"

namespace AxiomCommon {

    template<class InternalSequence, class FilterMapFunctor>
    using FilterMapWatchSequence = WatchSequence<FilterMapSequence<InternalSequence, FilterMapFunctor>>;

    template<class InternalSequence, class FilterFunctor>
    using FilterWatchSequence = WatchSequence<FilterSequence<InternalSequence, FilterFunctor>>;

    template<class InternalSequence, class MapFunctor>
    using MapWatchSequence = WatchSequence<MapSequence<InternalSequence, MapFunctor>>;

    template<class Output, class InternalSequence>
    using CastWatchSequence = WatchSequence<CastSequence<Output, InternalSequence>>;

    template<class Sequence, class FilterMapFunctor>
    FilterMapWatchSequence<Sequence, FilterMapFunctor> filterMapWatch(Sequence sequence, FilterMapFunctor functor) {
        return WatchSequence(filterMap(std::move(sequence), std::move(functor)));
    }

    template<class Sequence, class FilterFunctor>
    FilterWatchSequence<Sequence, FilterFunctor> filterWatch(Sequence sequence, FilterFunctor functor) {
        return WatchSequence(filter(std::move(sequence), std::move(functor)));
    }

    template<class Sequence, class MapFunctor>
    MapWatchSequence<Sequence, MapFunctor> mapWatch(Sequence sequence, MapFunctor functor) {
        return WatchSequence(map(std::move(sequence), std::move(functor)));
    }

    template<class Output, class Sequence>
    CastWatchSequence<Output, Sequence> dynamicCastWatch(Sequence sequence) {
        return WatchSequence(dynamicCast(std::move(sequence)));
    }

    template<class Output, class Sequence>
    CastWatchSequence<Output, Sequence> staticCastWatch(Sequence sequence) {
        return WatchSequence(staticCast(std::move(sequence)));
    }

    template<class Output, class Sequence>
    CastWatchSequence<Output, Sequence> reinterpretCastWatch(Sequence sequence) {
        return WatchSequence(reinterpretCast(std::move(sequence)));
    }
}
