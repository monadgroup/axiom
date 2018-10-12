#pragma once

#include <QtCore/QUuid>
#include <memory>
#include <unordered_set>

#include "PoolObject.h"
#include "common/WatchSequence.h"
#include "common/WatchSequenceOperators.h"

namespace AxiomModel {

    class PoolObject;

    class Pool {
        using BaseSequence = AxiomCommon::BaseWatchSequence<std::vector<std::unique_ptr<PoolObject>>>;

    public:
        using Sequence = AxiomCommon::CastWatchSequence<PoolObject *, BaseSequence>;

        Pool();

        virtual ~Pool();

        PoolObject *registerObj(std::unique_ptr<PoolObject> obj);

        std::unique_ptr<PoolObject> removeObj(PoolObject *obj);

        Sequence &sequence() { return _sequence; }

        const Sequence &sequence() const { return _sequence; }

        void destroy();

    private:
        BaseSequence _baseSequence;
        Sequence _sequence;
    };
}
