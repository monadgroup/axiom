#pragma once

#include <QtCore/QUuid>

namespace AxiomModel {

    class Pool;

    class PoolObject {
    public:
        PoolObject(QUuid uuid, QUuid parentUuid, Pool *pool);

        virtual ~PoolObject();

        const QUuid &uuid() const { return _uuid; }

        const QUuid &parentUuid() const { return _parentUuid; }

        Pool *pool() const { return _pool; }

    private:
        QUuid _uuid;

        QUuid _parentUuid;

        Pool *_pool;
    };

    class Test : public PoolObject {
    public:
        Test(Pool *pool) : PoolObject(QUuid(), QUuid(), pool) {}
    };

}
