#pragma once

#include <QtCore/QUuid>

#include "common/Hookable.h"

namespace AxiomModel {

    class Pool;

    class PoolObject : public AxiomCommon::Hookable {
    public:
        PoolObject(const QUuid &uuid, const QUuid &parentUuid, Pool *pool);

        const QUuid &uuid() const {
            return _uuid;
        }

        const QUuid &parentUuid() const {
            return _parentUuid;
        }

        Pool *pool() const {
            return _pool;
        }

        virtual void remove();

    private:
        QUuid _uuid;

        QUuid _parentUuid;

        Pool *_pool;
    };
}
