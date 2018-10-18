#pragma once

#include <functional>
#include <vector>

#include "SlotMap.h"

namespace AxiomCommon {

    class TrackedObject {
    public:
        TrackedObject() = default;

        TrackedObject(const TrackedObject &a) = delete;

        TrackedObject(TrackedObject &&a) = delete;

        TrackedObject &operator=(const TrackedObject &a) = delete;

        TrackedObject &operator=(TrackedObject &&a) = delete;

        virtual ~TrackedObject();

        void trackedObjectListenForRemove(TrackedObject *removeNotifier, size_t attachedData);

        virtual void trackedObjectNotifyRemove(TrackedObject *obj, size_t attachedData) {}

    private:
        struct RemoveHandler {
            TrackedObject *notifier;
            size_t attachedData;
        };
        std::vector<RemoveHandler> removeHandlers;
        std::vector<TrackedObject *> registeredEmitters;

        void stopListeningForRemove(TrackedObject *removeNotifier);
        void stopTrackingEmitter(TrackedObject *emitter);
    };
}
