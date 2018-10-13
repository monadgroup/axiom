#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "SlotMap.h"
#include "TrackedObject.h"

namespace AxiomCommon {

    template<class... Args>
    class Event : public TrackedObject {
    private:
        struct EventRef {
            Event *event;

            explicit EventRef(Event *event) : event(event) {}

            void operator()(const Args &... params) { (*event)(std::forward<const Args &>(params)...); }
        };

    public:
        using FuncType = std::function<void(Args...)>;
        using EventId = typename SlotMap<FuncType>::key;

        Event() = default;

        Event(Event &&) noexcept = default;

        Event &operator=(Event &&) noexcept = default;

        void operator()(const Args &... params) {
            for (const auto &connection : connections) {
                connection.value(params...);
            }
        }

        // call the provided function when the event is triggered
        EventId connect(FuncType listener) { return connections.insert(std::move(listener)); }

        // call the provided function when the event is triggered, automatically disconnecting when the
        // provided object is destructed
        EventId connect(TrackedObject *obj, FuncType listener) {
            auto eventId = connect(std::move(listener));
            obj->trackedObjectManager()->listenForRemove(obj->trackedObjectId(), trackedObjectId(), eventId);
            return eventId;
        }

        // call the provided function, passing in the follow instance even if it's moved
        template<class TB>
        EventId connect(TB *follow, std::function<void(TB *, Args...)> listener) {
            auto followId = follow->trackedObjectId();
            auto followManager = follow->trackedObjectManager();
            return connect(follow,
                           std::function<void(Args && ...)>([followId, followManager, listener](Args &&... params) {
                               auto followReified = (TB *) followManager->getObject(followId);
                               listener(followReified, std::forward<Args>(params)...);
                           }));
        }

        // call the provided method, automatically disconnecting when the base object is destructed
        template<class TB, class TFB, class TR, class... TA>
        EventId connect(TB *follow, TR (TFB::*listener)(TA...)) {
            return connect(follow, std::function<void(TB *, Args...)>([listener](TB *follow, Args... params) {
                               auto wrapper = std::mem_fn(listener);
                               applyFunc<sizeof...(TA) + 1>(wrapper, follow, std::forward<Args>(params)...);
                           }));
        }

        template<class TB, class TFB, class TR, class... TA>
        EventId connect(TB *follow, TR (TFB::*listener)(TA...) const) {
            return connect(follow, std::function<void(TB *, Args...)>([listener](TB *follow, Args... params) {
                               auto wrapper = std::mem_fn(listener);
                               applyFunc<sizeof...(TA) + 1>(wrapper, follow, std::forward<Args>(params)...);
                           }));
        }

        // trigger the provided event when this event is triggered
        EventId forward(Event *event) { return connect(event, std::function<void(const Args &...)>(EventRef(event))); }

        void disconnect(EventId event) { connections.erase(event); }

        void disconnectAll() { connections.clear(); }

        void trackedObjectNotifyRemove(TrackedObjectManager::ObjectId id, EventId eventId) override {
            disconnect(eventId);
        }

    private:
        SlotMap<FuncType> connections;

        // Utilities that allow us to call a function with more arguments than it needs
        // e.g. calling void myFunc(int) as myFunc(5, "hello", 2.6);
        // Useful for events, since often the event user doesn't care about some arguments.
        template<class Func, size_t... I, class... PassArgs>
        static void applyFuncIndexed(const Func &func, std::index_sequence<I...>, PassArgs &&... params) {
            func(std::get<I>(std::make_tuple(std::forward<PassArgs>(params)...))...);
        }

        template<size_t ArgCount, class Func, class... PassArgs>
        static void applyFunc(const Func &func, PassArgs &&... params) {
            applyFuncIndexed(func, std::make_index_sequence<ArgCount>{}, std::forward<PassArgs>(params)...);
        }
    };
}
