#include "ModelRoot.h"

#include <chrono>
#include <iostream>

#include "../backend/AudioBackend.h"
#include "IdentityReferenceMapper.h"
#include "ModelObject.h"
#include "PoolOperators.h"
#include "Project.h"
#include "editor/compiler/interface/Runtime.h"
#include "objects/Connection.h"
#include "objects/ControlSurface.h"
#include "objects/Node.h"
#include "objects/RootSurface.h"

using namespace AxiomModel;

ModelRoot::ModelRoot()
    : _history([this](std::vector<QUuid> items) { applyCompile(items); }),
      _nodeSurfaces(AxiomCommon::dynamicCastWatch<NodeSurface *>(_pool.sequence())),
      _nodes(AxiomCommon::dynamicCastWatch<Node *>(_pool.sequence())),
      _controlSurfaces(AxiomCommon::dynamicCastWatch<ControlSurface *>(_pool.sequence())),
      _controls(AxiomCommon::dynamicCastWatch<Control *>(_pool.sequence())),
      _connections(AxiomCommon::dynamicCastWatch<Connection *>(_pool.sequence())) {}

RootSurface *ModelRoot::rootSurface() {
    auto rootSurfaces = findChildren(nodeSurfaces().sequence(), QUuid());
    assert(rootSurfaces.size() == 1);
    auto rootSurface = dynamic_cast<RootSurface *>(*takeAt(rootSurfaces, 0));
    assert(rootSurface);
    return rootSurface;
}

void ModelRoot::attachRuntime(MaximCompiler::Runtime *runtime) {
    _runtime = runtime;

    MaximCompiler::Transaction buildTransaction;
    rootSurface()->attachRuntime(_runtime, &buildTransaction);
    applyTransaction(std::move(buildTransaction));
}

std::lock_guard<std::mutex> ModelRoot::lockRuntime() {
    return std::lock_guard(_runtimeLock);
}

void ModelRoot::applyItemsTo(const std::vector<QUuid> &items, MaximCompiler::Transaction *transaction) {
    if (items.empty()) return;

    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<ModelObject *> inputItems;
    QSet<QUuid> processedItems;
    std::deque<QUuid> addQueue(items.begin(), items.end());

    while (!addQueue.empty()) {
        auto item = addQueue.front();
        addQueue.pop_front();

        if (processedItems.contains(item)) continue;
        processedItems.insert(item);
        if (auto object = AxiomCommon::dynamicCast<ModelObject *>(pool().sequence().sequence()).find(item)) {
            inputItems.push_back(*object);

            for (const auto &linked : (*object)->compileLinks()) {
                addQueue.push_back(linked);
            }
        }
    }

    for (const auto &item : inputItems) {
        item->build(transaction);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    std::cout << "Transaction build took " << duration.count() / 1000000000. << "s" << std::endl;
}

void ModelRoot::applyCompile(const std::vector<QUuid> &items) {
    MaximCompiler::Transaction transaction;
    applyItemsTo(items, &transaction);
    applyTransaction(std::move(transaction));

    modified();
}

void ModelRoot::applyTransaction(MaximCompiler::Transaction transaction) {
    auto lock = lockRuntime();

    if (_runtime) {
        auto allObjects = AxiomCommon::dynamicCast<ModelObject *>(_pool.sequence().sequence());
        for (const auto &obj : allObjects) {
            obj->saveState();
        }

        _runtime->commit(std::move(transaction));
        rootSurface()->updateRuntimePointers(_runtime, _runtime->getRootPtr());

        for (const auto &obj : allObjects) {
            obj->restoreState();
        }
    }

    configurationChanged();
}

void ModelRoot::destroy() {
    _pool.destroy();
}
