#include "TrackedObject.h"

#include <iostream>

using namespace AxiomCommon;

TrackedObjectManager TrackedObjectManager::main;

bool TrackedObjectManager::objectExists(AxiomCommon::TrackedObjectManager::ObjectId id) {
    return map.find(id) != map.end();
}

void TrackedObjectManager::allocateTrackedObject(AxiomCommon::TrackedObjectManager::ObjectId id) {
    map.emplace(id, MapEntry());
    std::cout << map.size() << " TrackedObjects allocated after adding" << std::endl;
}

void TrackedObjectManager::removeTrackedObject(AxiomCommon::TrackedObjectManager::ObjectId id) {
    auto iter = map.find(id);
    auto removeHandlers = std::move(iter->second.removeHandlers);
    for (const auto &removeHandler : removeHandlers) {
        if (!objectExists(removeHandler.notifier)) continue;
        removeHandler.notifier->trackedObjectNotifyRemove(id, removeHandler.attachedData);
    }
    map.erase(iter);
    std::cout << map.size() << " TrackedObjects allocated after removing" << std::endl;
}

void TrackedObjectManager::listenForRemove(AxiomCommon::TrackedObjectManager::ObjectId listenId,
                                           TrackedObject *removeNotifierId, size_t attachedData) {
    map[listenId].removeHandlers.push_back({removeNotifierId, attachedData});
}

TrackedObject::TrackedObject(AxiomCommon::TrackedObjectManager *manager) : manager(manager) {
    manager->allocateTrackedObject(this);
}

TrackedObject::~TrackedObject() {
    manager->removeTrackedObject(this);
}
