#pragma once

#include <functional>
#include <vector>

#include "SlotMap.h"

namespace AxiomCommon {

    class TrackedObject;

    class TrackedObjectManager {
    public:
    private:
        struct RemoveHandler {
            size_t notifier;
            size_t attachedData;
        };

        struct MapEntry {
            TrackedObject *obj;
            std::vector<RemoveHandler> removeHandlers;
        };

    public:
        using ObjectId = SlotMap<MapEntry>::key;

        static TrackedObjectManager main;

        TrackedObject *getObject(ObjectId id);

        // not part of the public API, used by TrackedObject
        ObjectId allocateTrackedObject(TrackedObject *);
        void moveTrackedObject(ObjectId id, TrackedObject *);
        void removeTrackedObject(ObjectId id);

        void listenForRemove(ObjectId listenId, ObjectId removeNotifierId, size_t attachedData);

    private:
        SlotMap<MapEntry> map;
    };

    class TrackedObject {
    public:
        explicit TrackedObject(TrackedObjectManager *manager = &TrackedObjectManager::main);

        TrackedObject(const TrackedObject &a) = delete;

        TrackedObject(TrackedObject &&a) noexcept;

        TrackedObject &operator=(const TrackedObject &a) = delete;

        TrackedObject &operator=(TrackedObject &&a) noexcept;

        virtual ~TrackedObject();

        TrackedObjectManager *trackedObjectManager() const { return manager; }

        TrackedObjectManager::ObjectId trackedObjectId() const { return objectId; }

        virtual void trackedObjectNotifyRemove(TrackedObjectManager::ObjectId, size_t) {}

    private:
        bool hasMoved = false;
        TrackedObjectManager *manager;
        TrackedObjectManager::ObjectId objectId;
    };
}
