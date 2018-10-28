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
    : _nodeSurfaces(AxiomCommon::dynamicCastWatch<NodeSurface *>(_pool.sequence())),
      _nodes(AxiomCommon::dynamicCastWatch<Node *>(_pool.sequence())),
      _controlSurfaces(AxiomCommon::dynamicCastWatch<ControlSurface *>(_pool.sequence())),
      _controls(AxiomCommon::dynamicCastWatch<Control *>(_pool.sequence())),
      _connections(AxiomCommon::dynamicCastWatch<Connection *>(_pool.sequence())) {
    _history.stackChanged.connect(this, &ModelRoot::compileDirtyItems);
}

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

    // clear the dirty state of everything, since we've just compiled them
    for (const auto &obj : pool().sequence().sequence()) {
        if (auto modelObj = dynamic_cast<ModelObject *>(obj)) {
            modelObj->clearDirty();
        }
    }
}

std::lock_guard<std::mutex> ModelRoot::lockRuntime() {
    return std::lock_guard(_runtimeLock);
}

void ModelRoot::setHistory(AxiomModel::HistoryList history) {
    _history = std::move(history);
    _history.stackChanged.connect(this, &ModelRoot::compileDirtyItems);
}

void ModelRoot::applyDirtyItemsTo(MaximCompiler::Transaction *transaction) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // iterate over pool items in reverse order, since we need to compile children before parents
    size_t dirtyItemCount = 0;
    for (auto rit = pool().objects().rbegin(); rit < pool().objects().rend(); rit++) {
        auto obj = dynamic_cast<ModelObject *>(rit->get());
        if (obj && obj->isDirty()) {
            obj->clearDirty();
            obj->build(transaction);
            dirtyItemCount++;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    std::cout << "Transaction build (" << dirtyItemCount << " items"
              << ") took " << duration.count() / 1000000000. << "s" << std::endl;
}

void ModelRoot::compileDirtyItems() {
    MaximCompiler::Transaction transaction;
    applyDirtyItemsTo(&transaction);
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
