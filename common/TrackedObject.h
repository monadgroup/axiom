#pragma once

#include <functional>
#include <vector>

#include "SlotMap.h"

namespace AxiomCommon {

    class TrackedObject;

    class TrackedObjectManager {
    public:
        using ObjectId = const TrackedObject *;

    private:
        struct RemoveHandler {
            TrackedObject *notifier;
            size_t attachedData;
        };

        struct MapEntry {
            std::vector<RemoveHandler> removeHandlers;
        };

    public:
        static TrackedObjectManager main;

        bool objectExists(ObjectId id);

        // not part of the public API, used by TrackedObject
        void allocateTrackedObject(ObjectId id);
        void removeTrackedObject(ObjectId id);

        void listenForRemove(ObjectId listenId, TrackedObject *removeNotifierId, size_t attachedData);

    private:
        std::unordered_map<ObjectId, MapEntry> map;
    };

    class TrackedObject {
    public:
        explicit TrackedObject(TrackedObjectManager *manager = &TrackedObjectManager::main);

        TrackedObject(const TrackedObject &a) = delete;

        TrackedObject(TrackedObject &&a) = delete;

        TrackedObject &operator=(const TrackedObject &a) = delete;

        TrackedObject &operator=(TrackedObject &&a) = delete;

        virtual ~TrackedObject();

        TrackedObjectManager *trackedObjectManager() const { return manager; }

        TrackedObjectManager::ObjectId trackedObjectId() const { return this; }

        virtual void trackedObjectNotifyRemove(TrackedObjectManager::ObjectId, size_t) {}

    private:
        TrackedObjectManager *manager;
    };
}
