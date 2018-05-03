#pragma once

#include <QtCore/QUuid>

#include "Hookable.h"

namespace AxiomModel {

    class Pool;

    class PoolObject : public Hookable {
    public:
        PoolObject(const QUuid &uuid, const QUuid &parentUuid, Pool *pool);

        ~PoolObject() override;

        const QUuid &uuid() const { return _uuid; }

        const QUuid &parentUuid() const { return _parentUuid; }

        Pool *pool() const { return _pool; }

    private:
        QUuid _uuid;

        QUuid _parentUuid;

        Pool *_pool;
    };

}
