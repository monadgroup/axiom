#pragma once

#include <QtCore/QUuid>
#include <memory>
#include <unordered_set>

#include "IndexedSequence.h"
#include "PoolObject.h"
#include "common/WatchSequence.h"
#include "common/WatchSequenceOperators.h"

namespace AxiomModel {

    class PoolObject;

    class Pool {
        using IterSequence = AxiomCommon::IterSequence<std::vector<std::unique_ptr<PoolObject>>>;
        using BaseSequence = AxiomCommon::BaseWatchSequence<
            IndexedSequence<AxiomCommon::MapSequence<IterSequence, PoolObject *(*) (std::unique_ptr<PoolObject> *)>>,
            AxiomCommon::BaseWatchEvents<PoolObject *>>;

    public:
        using Sequence = AxiomCommon::RefWatchSequence<BaseSequence>;

        Pool();

        virtual ~Pool();

        PoolObject *registerObj(std::unique_ptr<PoolObject> obj);

        std::unique_ptr<PoolObject> removeObj(PoolObject *obj);

        const std::vector<std::unique_ptr<PoolObject>> &objects() const { return _objects; }

        Sequence sequence() { return AxiomCommon::refWatchSequence(&_sequence); }

        void destroy();

    private:
        std::vector<std::unique_ptr<PoolObject>> _objects;
        QHash<QUuid, PoolObject *> index;
        BaseSequence _sequence;
    };
}
