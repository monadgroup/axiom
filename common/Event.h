#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "SlotMap.h"
#include "TrackedObject.h"

namespace AxiomCommon {

    template<class... Args>
    class Event {
    public:
        using FuncType = std::function<void(Args...)>;
        using EventId = typename SlotMap<FuncType>::key;

    private:
        struct EventRef {
            Event *event;

            explicit EventRef(Event *event) : event(event) {}

            void operator()(const Args &... params) { (*event)(std::forward<const Args &>(params)...); }
        };

        struct EventData : public TrackedObject {
            SlotMap<FuncType> connections;
            bool isDispatching = false;
            std::unique_ptr<EventData> queuedDestroy;

            void handleDestroy(std::unique_ptr<EventData> ownership) {
                if (isDispatching) {
                    queuedDestroy = std::move(ownership);
                }
            }

            void dispatch(const Args &... params) {
                isDispatching = true;
                for (const auto &connection : connections) {
                    connection.value(params...);
                }
                isDispatching = false;

                // if the event was destroyed during the dispatch, destroy it now
                // note: this destroys `this`!
                if (queuedDestroy) {
                    queuedDestroy.reset();
                }
            }

            void disconnect(EventId event) { connections.erase(event); }

            void trackedObjectNotifyRemove(TrackedObject *, EventId eventId) override { disconnect(eventId); }
        };

    public:
        struct EventTarget {
            TrackedObject *bindObj;
            FuncType listener;

            EventTarget(TrackedObject *bindObj, FuncType listener) : bindObj(bindObj), listener(std::move(listener)) {}
        };

        Event() : data(std::make_unique<EventData>()) {}

        Event(Event &&a) noexcept = default;

        ~Event() {
            if (data) {
                auto dataPtr = data.get();
                dataPtr->handleDestroy(std::move(data));
            }
        }

        Event &operator=(Event &&a) noexcept = default;

        void operator()(const Args &... params) { data->dispatch(params...); }

        static EventTarget to(FuncType listener) { return EventTarget(nullptr, std::move(listener)); }

        static EventTarget to(TrackedObject *obj, FuncType listener) { return EventTarget(obj, std::move(listener)); }

        template<class TB, class TFB, class TR, class... TA>
        static EventTarget to(TB *follow, TR (TFB::*listener)(TA...)) {
            return to(follow, [follow, listener](Args... params) {
                auto wrapper = std::mem_fn(listener);
                applyFunc<sizeof...(TA) + 1>(wrapper, follow, std::forward<Args>(params)...);
            });
        }

        template<class TB, class TFB, class TR, class... TA>
        static EventTarget to(TB *follow, TR (TFB::*listener)(TA...) const) {
            return to(follow, [follow, listener](Args... params) {
                auto wrapper = std::mem_fn(listener);
                applyFunc<sizeof...(TA) + 1>(wrapper, follow, std::forward<Args>(params)...);
            });
        }

        static EventTarget to(Event *event) {
            // data is guaranteed to not move
            auto dataPtr = event->data.get();
            return to(dataPtr, [dataPtr](Args... params) { dataPtr->dispatch(params...); });
        }

        EventId connect(EventTarget target) {
            auto eventId = data->connections.insert(std::move(target.listener));
            if (target.bindObj) {
                target.bindObj->trackedObjectListenForRemove(data.get(), eventId);
            }
            return eventId;
        }

        template<class... ToArgs>
        EventId connectTo(ToArgs &&... args) {
            return connect(to(std::forward<ToArgs>(args)...));
        }

        template<class Func>
        EventId connectWith(Func func) {
            return data->connections.insertWith([this, func](EventId eventId) {
                auto target = func(eventId);
                if (target.bindObj) {
                    target.bindObj->trackedObjectListenForRemove(data.get(), eventId);
                }
                return std::move(target.listener);
            });
        }

        EventId once(EventTarget target) {
            auto dataPtr = data.get();
            return connectWith([dataPtr, target](EventId eventId) {
                auto targetListener = std::move(target.listener);
                return EventTarget(target.bindObj, [dataPtr, eventId, targetListener](Args... params) {
                    // move everything out of the closure context, since it might be destroyed by the end
                    // of the function.
                    auto data = dataPtr;
                    auto event = eventId;
                    auto listener = std::move(targetListener);

                    listener(std::forward<Args>(params)...);
                    data->disconnect(event);
                });
            });
        }

        void disconnect(EventId event) { data->disconnect(event); }

        void disconnectAll() { data->connections.clear(); }

    private:
        std::unique_ptr<EventData> data;

        // Utilities that allow us to call a function with more arguments than it needs
        // e.g. calling void myFunc(int) as myFunc(5, "hello", 2.6);
        // Useful for events, since often the event user doesn't care about some arguments.
        template<class Func, size_t... I, class... PassArgs>
        static void applyFuncIndexed(const Func &func, std::index_sequence<I...>, PassArgs &&... params) {
            func(std::get<I>(std::make_tuple(std::forward<PassArgs>(params)...))...);
        }

        template<size_t ArgCount, class Func, class... PassArgs>
        static void applyFunc(const Func &func, PassArgs &&... params) {
            applyFuncIndexed(func, std::make_index_sequence<ArgCount> {}, std::forward<PassArgs>(params)...);
        }
    };
}
