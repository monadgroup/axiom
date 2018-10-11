#include "TrackedObject.h"

#include <iostream>

using namespace AxiomCommon;

TrackedObjectManager TrackedObjectManager::main;

TrackedObject *TrackedObjectManager::getObject(TrackedObjectManager::ObjectId id) {
    return map[id].obj;
}

TrackedObjectManager::ObjectId TrackedObjectManager::allocateTrackedObject(TrackedObject *obj) {
    std::cout << "Allocating TrackedObject, now have " << map.size() + 1 << std::endl;
    return map.insert({obj, {}});
}

void TrackedObjectManager::moveTrackedObject(TrackedObjectManager::ObjectId id, TrackedObject *obj) {
    map[id].obj = obj;
}

void TrackedObjectManager::removeTrackedObject(TrackedObjectManager::ObjectId id) {
    auto removeHandlers = std::move(map[id].removeHandlers);
    for (const auto &removeHandler : removeHandlers) {
        auto targetObj = map.find(removeHandler.notifier);
        if (targetObj != map.end()) targetObj->obj->trackedObjectNotifyRemove(id, removeHandler.attachedData);
    }
    map.erase(id);
    std::cout << "Removing TrackedObject, now have " << map.size() << std::endl;
}

void TrackedObjectManager::listenForRemove(AxiomCommon::TrackedObjectManager::ObjectId listenId,
                                           AxiomCommon::TrackedObjectManager::ObjectId removeNotifierId,
                                           size_t attachedData) {
    map[listenId].removeHandlers.push_back({removeNotifierId, attachedData});
}

TrackedObject::TrackedObject(TrackedObjectManager *manager)
    : manager(manager), objectId(manager->allocateTrackedObject(this)) {}

TrackedObject::TrackedObject(TrackedObject &&a) noexcept : manager(a.manager), objectId(a.objectId) {
    a.hasMoved = true;
    manager->moveTrackedObject(objectId, this);
}

TrackedObject &TrackedObject::operator=(TrackedObject &&a) noexcept {
    a.hasMoved = true;
    manager = a.manager;
    objectId = a.objectId;
    manager->moveTrackedObject(objectId, this);
    return *this;
}

TrackedObject::~TrackedObject() {
    if (!hasMoved) {
        manager->removeTrackedObject(objectId);
    }
}
