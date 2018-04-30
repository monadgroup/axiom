#include "Pool.h"

#include "PoolObject.h"

using namespace AxiomModel;

Pool::Pool() : PoolView(_objects, [](const collection_value_type &base) -> std::optional<value_type> { return base; }) {}

void Pool::registerObj(AxiomModel::PoolObject *obj) {
    _objects.emplace(obj);
}

void Pool::removeObj(AxiomModel::PoolObject *obj) {
    _objects.erase(obj);
}

void bla() {
    Pool myPool;
    QUuid parentId;

    auto testItems = myPool.filterType<Test>();
    auto testChildren = testItems.filterChildren(parentId);

    for (const auto &child : testChildren) {
        // `child` is an instance of `Test` that has it's parent UUID set to `parentId`
    }

    testChildren.filter(testItems.filter(*myPool.collection().begin()));

    //testChildren.filter(testItems.filter(*myPool.collection().begin()));

    //auto rootItem = testChildren.filter

    //auto rootItem = testChildren.filter
}
