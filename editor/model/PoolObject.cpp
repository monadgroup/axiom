#include "PoolObject.h"

#include "Pool.h"

using namespace AxiomModel;

PoolObject::PoolObject(const QUuid &uuid, const QUuid &parentUuid, AxiomModel::Pool *pool)
    : _uuid(uuid), _parentUuid(parentUuid), _pool(pool) {}

void PoolObject::remove() {
    _pool->removeObj(this);
}
