#include "TrackedObject.h"

#include <iostream>

using namespace AxiomCommon;

TrackedObjectManager TrackedObjectManager::main;

TrackedObject *TrackedObjectManager::getObject(TrackedObjectManager::ObjectId id) {
    return map[id].obj;
}

TrackedObjectManager::ObjectId TrackedObjectManager::allocateTrackedObject(TrackedObject *obj) {
    auto insertId = map.insert({obj, {}});
    std::cout << "Allocating TrackedObject with ID " << insertId << ", now have " << map.size() << std::endl;
    return insertId;
}

void TrackedObjectManager::moveTrackedObject(TrackedObjectManager::ObjectId id, TrackedObject *obj) {
    std::cout << "Moving " << id << " to " << obj << std::endl;
    map[id].obj = obj;
}

void TrackedObjectManager::removeTrackedObject(TrackedObjectManager::ObjectId id) {
    std::cout << "Started removing TrackedObject with ID " << id << std::endl;
    auto removeHandlers = std::move(map[id].removeHandlers);
    for (const auto &removeHandler : removeHandlers) {
        auto targetObj = map.find(removeHandler.notifier);
        if (targetObj != map.end()) {
            auto targetObjAddress = targetObj->obj;
            targetObjAddress->trackedObjectNotifyRemove(id, removeHandler.attachedData);
        }
    }
    map.erase(id);
    std::cout << "Removing TrackedObject with ID " << id << ", now have " << map.size() << std::endl;
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
    hasMoved = false;
    manager->moveTrackedObject(objectId, this);
}

TrackedObject &TrackedObject::operator=(TrackedObject &&a) noexcept {
    a.hasMoved = true;
    hasMoved = false;
    manager = a.manager;
    objectId = a.objectId;
    manager->moveTrackedObject(objectId, this);
    return *this;
}

TrackedObject::~TrackedObject() {
    if (!hasMoved) {
        manager->removeTrackedObject(objectId);
    } else {
        std::cout << "Not removing object at " << this << " with ID " << objectId << std::endl;
    }
}
