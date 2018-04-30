#include "PoolObject.h"
#include "Pool.h"

using namespace AxiomModel;

PoolObject::PoolObject(QUuid uuid, QUuid parentUuid, AxiomModel::Pool *pool) : _uuid(uuid), _parentUuid(parentUuid), _pool(pool) {
    _pool->registerObj(this);
}

PoolObject::~PoolObject() {
    _pool->removeObj(this);
}
