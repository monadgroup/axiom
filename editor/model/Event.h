#pragma once

#include <vector>
#include <optional>
#include <functional>
#include <set>
#include <utility>
#include <cassert>
#include <tuple>
#include <memory>

#include "Hookable.h"

namespace AxiomModel {

    template<class... Args>
    class Event : public Hookable {
    public:
        using func_type = std::function<void(Args...)>;

    private:
        struct SharedEvent;

        using shared_event = std::shared_ptr<SharedEvent>;

        class SharedEvent : public Hookable, public HookNotifiable {
        public:
            std::optional<func_type> callback;
            std::vector<shared_event> children;
            std::weak_ptr<SharedEvent> parent;
            std::set<Hookable*> following;

            SharedEvent() {}

            ~SharedEvent() override {
                removeHooks();
            }

            explicit SharedEvent(func_type callback) : callback(callback) {}

            void trigger(const Args&... params) const {
                if (callback) {
                    (*callback)(params...);
                }

                for (const auto &child : children) {
                    child->trigger(params...);
                }
            }

            void disconnect(SharedEvent *other) {
                for (auto i = children.begin(); i != children.end(); i++) {
                    if (i->get() != other) continue;

                    other->parent.reset();
                    children.erase(i);
                    return;
                }
                assert(false);
            }

            void follow(Hookable *hookable) {
                hookable->addDestructHook(this);
                following.emplace(hookable);
            }

            void unfollow(Hookable *hookable) {
                hookable->removeDestructHook(this);
                following.erase(hookable);
            }

            void hookableDestroyed(Hookable *hookable) override {
                following.erase(hookable);
                detach();
            }

            void detach() {
                callback.reset();
                children.clear();
                removeHooks();
                following.clear();

                if (!parent.expired()) {
                    parent.lock()->disconnect(this);
                }
            }

            void removeHooks() {
                for (const auto &follow : following) {
                    follow->removeDestructHook(this);
                }
            }
        };

    public:
        Event() : impl(std::make_shared<SharedEvent>()) {}

        explicit Event(func_type callback) : impl(std::make_shared<SharedEvent>(callback)) {}

        bool operator==(const Event &other) const { return impl == other.impl; }

        bool operator!=(const Event &other) const { return impl != other.impl; }

        void trigger(const Args&... params) const {
            impl->trigger(params...);
        }

        Event connect(Event listener) {
            impl->children.push_back(listener.impl);
            listener.impl->parent = impl;
            return listener;
        }

        Event connect(Event *other) {
            auto evt = connect(Event(std::function([other](Args&&... params) {
                other->trigger(std::forward<Args>(params)...);
            })));
            evt.follow(other);
            return evt;
        }

        Event connect(func_type listener) {
            return connect(Event(listener));
        }

        Event connect(Hookable *follow, Event *listener) {
            auto link = connect(listener);
            link.follow(follow);
            return link;
        }

        Event connect(Hookable *follow, Event listener) {
            auto link = connect(std::move(listener));
            link.follow(follow);
            return link;
        }

        Event connect(Hookable *follow, func_type listener) {
            return connect(follow, Event(listener));
        }

        template<class TR, class... TA>
        Event connect(Hookable *follow, std::function<TR(TA...)> listener) {
            return connect(follow, Event(std::function([listener](Args&&... params) {
                applyFunc<sizeof...(TA)>(listener, std::forward<Args>(params)...);
            })));
        }

        template<class TB, class TFB, class TR, class... TA>
        Event connect(TB *follow, TR (TFB::*listener)(TA...)) {
            auto wrapper = std::mem_fn(listener);
            return connect(follow, Event(std::function([follow, wrapper](Args&&... params) {
                applyFunc<sizeof...(TA) + 1>(wrapper, follow, std::forward<Args>(params)...);
            })));
        }

        template<class TB, class TFB, class TR, class... TA>
        Event forward(TB *object, TR (TFB::*listener)(TA...)) {
            auto wrapper = std::mem_fn(listener);
            return connect(Event(std::function([object, wrapper](Args&&... params) {
                applyFunc<sizeof...(TA) + 1>(wrapper, object, std::forward<Args>(params)...);
            })));
        }

        void disconnect(Event other) {
            impl->disconnect(other.impl.get());
        }

        void follow(Hookable *hookable) {
            impl->follow(hookable);
        }

        void unfollow(Hookable *hookable) {
            impl->unfollow(hookable);
        }

        void detach() {
            impl->detach();
        }

        bool empty() const {
            return impl->children.empty();
        }

        size_t size() const {
            return impl->children.size();
        }

    private:
        shared_event impl;

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
