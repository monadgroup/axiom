#pragma once

#include <vector>
#include <optional>
#include <functional>
#include <set>
#include <utility>
#include <cassert>

#include "Hookable.h"

namespace AxiomModel {

    template<class... Args>
    class Event : public Hookable {
    public:
        using func_type = std::function<void(Args...)>;

        Event() noexcept = default;

        explicit Event(func_type func) noexcept : callback(func) {}

        Event(const Event &a) = delete;

        Event &operator=(const Event &a) = delete;

        Event(Event &&a) noexcept {
            callback = std::move(a.callback);
            listeners = std::move(a.listeners);
        }

        Event &operator=(Event &&a) noexcept {
            triggerDestruct(false);
            callback = std::move(a.callback);
            listeners = std::move(a.listeners);
            return *this;
        }

        void trigger(Args... params) const {
            if (callback) {
                (*callback)(params...);
            }
        }

        Event *connect(Event *other) {
            auto ptr = connect(Event(std::function([other](Args&&... params) {
                other->trigger(std::forward<Args>(params)...);
            })));
            ptr->follow(other);
            return ptr;
        }

        Event *connect(Event listener) {
            listeners.push_back(std::move(listener));
            auto ptr = &listeners.back();
            ptr->parent = this;
            return ptr;
        }

        Event *connect(func_type listener) {
            return connect(Event(listener));
        }

        Event *connect(Hookable *follow, Event *listener) {
            auto link = connect(listener);
            link->follow(follow);
            return link;
        }

        Event *connect(Hookable *follow, Event listener) {
            auto link = connect(std::move(listener));
            link->follow(follow);
            return link;
        }

        Event *connect(Hookable *follow, func_type listener) {
            return connect(follow, Event(listener));
        }

        template<class TR, class... TA>
        Event *connect(Hookable *follow, std::function<TR(TA...)> listener) {
            return connect(follow, Event(std::function([listener](Args&&... params) {
                applyFunc<sizeof...(TA)>(listener, std::forward<Args>(params)...);
            })));
        };

        template<class TB, class TFB, class TR, class... TA>
        Event *connect(TB *follow, TR (TFB::*listener)(TA...)) {
            auto wrapper = std::mem_fn(listener);
            return connect(follow, Event(std::function([follow, wrapper](Args&&... params) {
                applyFunc<sizeof...(TA) + 1>(wrapper, follow, std::forward<Args>(params)...);
            })));
        };

        template<class TB, class TFB, class TR, class... TA>
        Event *forward(TB *object, TR (TFB::*listener)(TA...)) {
            auto wrapper = std::mem_fn(listener);
            return connect(Event(std::function([object, wrapper](Args&&... params) {
                applyFunc<sizeof...(TA) + 1>(wrapper, object, std::forward<Args>(params)...);
            })));
        };

        void disconnect(Event *other) {
            auto iter = typename std::vector<Event>::iterator(other);
            assert(iter >= listeners.begin() && iter < listeners.end());

            // ensure the listener doesn't try and detach itself
            other->parent = nullptr;
            listeners.erase(iter);
        }

        void follow(Hookable *other) {
            following.emplace(other);
            other->addDestructHook(this, [this]() { triggerDestruct(false); });
        }

        void unfollow(Hookable *other) {
            following.erase(other);
            other->removeDestructHook(this);
        }

    protected:
        void triggerDestruct(bool isFinal) override {
            if (parent) {
                // this will cause disconnect() to be called on the parent, resetting our parent ptr
                // and triggering the code in else{}
                parent->disconnect(this);
            } else {
                while (!following.empty()) {
                    unfollow(*following.begin());
                }
                Hookable::triggerDestruct(isFinal);
            }
        }

    private:
        std::optional<func_type> callback;
        Event* parent = nullptr;
        std::vector<Event> listeners;
        std::set<Hookable*> following;

        template<class Func, size_t... I, class... PassArgs>
        static void applyFuncIndexed(const Func &func, std::index_sequence<I...>, PassArgs&&... params) {
            func(std::get<I>(std::make_tuple(std::forward<PassArgs>(params)...))...);
        }

        template<size_t ArgCount, class Func, class... PassArgs>
        static void applyFunc(const Func &func, PassArgs&&... params) {
            applyFuncIndexed(func, std::make_index_sequence<ArgCount>{}, std::forward<PassArgs>(params)...);
        }
    };

}
