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
    class Event : public Hookable, public HookNotifiable {
    public:
        using func_type = std::function<void(Args...)>;

        Event() {}

        explicit Event(func_type callback) : callback(callback) {}

        Event(const Event &a) {
            doCopy(a);
        }

        Event(Event &&a) noexcept : Hookable(std::move(a)) {
            doMove(a);
        }

        Event &operator=(const Event &a) {
            Hookable::operator=(a);
            destruct();
            cleanup();
            doCopy(a);

            return *this;
        }

        Event &operator=(Event &&a) noexcept {
            Hookable::operator=(std::move(a));
            destruct();
            cleanup();
            doMove(a);

            return *this;
        }

        ~Event() override {
            destruct();
        }

        void trigger(const Args&... params) const {
            if (callback) {
                (*callback)(params...);
            }

            // todo: handle children being added and removed from the array while we're looping
            for (const auto &child : children) {
                child.trigger(params...);
            }
        }

        Event *connect(Event listener) {
            children.push_back(std::move(listener));
            auto ptr = &children.back();
            ptr->parent = this;
            return ptr;
        }

        Event *connect(Event *other) {
            auto ptr = connect(Event(std::function([other](Args&&... params) {
                other->trigger(std::forward<Args>(params)...);
            })));
            ptr->follow(other);
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

        // note: `other` will be invalidated by calling this method
        void disconnect(Event *other) {
            destroyChild(other);
        }

        void follow(Hookable *hookable) {
            hookable->addDestructHook(this);
            following.emplace(hookable);
        }

        void unfollow(Hookable *hookable) {
            hookable->removeDestructHook(this);
            following.erase(hookable);
        }

        // this is called when any hookables we're following are destructed
        void hookableDestroyed(Hookable *hookable) override {
            following.erase(hookable);
            destroy();
        }

    private:
        std::optional<func_type> callback;
        std::vector<Event> children;
        Event *parent = nullptr;
        std::set<Hookable*> following;

        void destroyChild(Event *child) {
            auto iter = typename std::vector<Event>::iterator(child);
            assert(iter >= children.begin() && iter < children.end());
            child->parent = nullptr;
            children.erase(iter);
        }

        void destroy() {
            assert(parent);

            // this is going to end up calling our constructor
            parent->destroyChild(this);
        }

        void destruct() {
            //assert(!parent);

            // remove ourselves from any hookables we're following
            for (const auto &hookable : following) {
                hookable->removeDestructHook(this);
            }
        }

        void cleanup() {
            children.clear();
            following.clear();
            parent = nullptr;
            callback.reset();
        }

        void doCopy(const Event &a) {
            // todo: do we want to copy children and following?
            callback = a.callback;
        }

        void doMove(Event &a) {
            callback = std::move(a.callback);
            children = std::move(a.children);
            for (const auto &hookable : a.following) {
                hookable->removeDestructHook(&a);
                follow(hookable);
            }
            a.following.clear();

            if (a.parent) {
                parent = a.parent;

                // todo: is anything else needed to cleanup a?
            }
        }

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
