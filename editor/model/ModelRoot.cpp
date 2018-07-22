#include "ModelRoot.h"

#include "../backend/AudioBackend.h"
#include "IdentityReferenceMapper.h"
#include "ModelObject.h"
#include "PoolOperators.h"
#include "editor/compiler/interface/Runtime.h"
#include "objects/Connection.h"
#include "objects/ControlSurface.h"
#include "objects/Node.h"
#include "objects/RootSurface.h"

using namespace AxiomModel;

ModelRoot::ModelRoot(Project *project)
    : _project(project), _history([this](std::vector<QUuid> items) { applyCompile(items); }),
      _nodeSurfaces(dynamicCastWatch<NodeSurface *>(_pool.sequence())),
      _nodes(dynamicCastWatch<Node *>(_pool.sequence())),
      _controlSurfaces(dynamicCastWatch<ControlSurface *>(_pool.sequence())),
      _controls(dynamicCastWatch<Control *>(_pool.sequence())),
      _connections(dynamicCastWatch<Connection *>(_pool.sequence())) {}

RootSurface *ModelRoot::rootSurface() const {
    auto rootSurfaces = findChildren(nodeSurfaces(), QUuid());
    assert(rootSurfaces.size() == 1);
    auto rootSurface = dynamic_cast<RootSurface *>(takeAt(rootSurfaces, 0));
    assert(rootSurface);
    return rootSurface;
}

void ModelRoot::attachBackend(AxiomBackend::AudioBackend *backend) {
    _backend = backend;
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
    QSet<QUuid> processedItems;
    for (const auto &item : items) {
        if (processedItems.contains(item)) continue;
        processedItems.insert(item);

        if (auto object = findMaybe<ModelObject *>(pool().sequence(), item)) {
            (*object)->build(transaction);
        }
    }
}

void ModelRoot::applyCompile(const std::vector<QUuid> &items) {
    MaximCompiler::Transaction transaction;
    applyItemsTo(items, &transaction);
    applyTransaction(std::move(transaction));
}

void ModelRoot::applyTransaction(MaximCompiler::Transaction transaction) {
    auto lock = lockRuntime();

    if (_runtime) {
        _runtime->commit(std::move(transaction));
        rootSurface()->updateRuntimePointers(_runtime, _runtime->getRootPtr());
    }
    if (_backend) {
        _backend->internalUpdateConfiguration();
    }
}

void ModelRoot::destroy() {
    _pool.destroy();
}
