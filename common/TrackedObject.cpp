#include "TrackedObject.h"

#include <iostream>

using namespace AxiomCommon;

TrackedObject::~TrackedObject() {
    auto handlers = std::move(removeHandlers);
    for (const auto &handler : handlers) {
        handler.notifier->trackedObjectNotifyRemove(this, handler.attachedData);
        handler.notifier->stopTrackingEmitter(this);
    }

    auto emitters = std::move(registeredEmitters);
    for (const auto &emitter : emitters) {
        emitter->stopListeningForRemove(this);
    }
}

void TrackedObject::trackedObjectListenForRemove(AxiomCommon::TrackedObject *removeNotifier, size_t attachedData) {
    removeHandlers.push_back({removeNotifier, attachedData});
    removeNotifier->registeredEmitters.push_back(this);
}

void TrackedObject::stopListeningForRemove(AxiomCommon::TrackedObject *removeNotifier) {
    for (auto it = removeHandlers.begin(); it != removeHandlers.end(); ++it) {
        if (it->notifier == removeNotifier) {
            removeHandlers.erase(it);
            --it;
        }
    }
}

void TrackedObject::stopTrackingEmitter(AxiomCommon::TrackedObject *emitter) {
    for (auto it = registeredEmitters.begin(); it != registeredEmitters.end(); ++it) {
        if (*it == emitter) {
            registeredEmitters.erase(it);
            --it;
        }
    }
}
