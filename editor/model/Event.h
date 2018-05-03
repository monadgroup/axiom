#pragma once

#include <optional>
#include <functional>
#include <set>

namespace AxiomModel {

    template<class... A>
    class Event {
    private:
        using owned_collection = typename std::vector<Event>;

    public:
        using func_type = std::function<void(A...)>;

        Event() noexcept = default;

        explicit Event(func_type func) noexcept : callback(func) {}

        Event(const Event &a) noexcept : callback(a.callback) {
            copyFrom(a);
        }

        Event(Event &&a) noexcept {
            moveFrom(a);
        }

        Event &operator=(const Event &a) noexcept {
            detachAll();
            copyFrom(a);
            return *this;
        }

        Event &operator=(Event &&a) noexcept {
            detachAll();
            moveFrom(a);
            return *this;
        }

        ~Event() {
            detachAll();
        }

        void emit(A... params) const {
            if (callback) {
                (*callback)(params...);
            }

            for (const auto &listener : listeners) {
                listener->emit(params...);
            }
        }

        Event *listen(Event listener) {
            ownedListeners.push_back(std::move(listener));
            auto ptr = &ownedListeners.back();
            connect(ptr);
            return ptr;
        }

        Event *listen(func_type listener) {
            return listen(Event(listener));
        }

        Event *listen(Event *follow, Event listener) {
            auto result = listen(std::move(listener));
            result->follow(follow);
            return result;
        }

        Event *listen(Event *follow, func_type listener) {
            return listen(follow, Event(listener));
        }

        void connect(Event *other) {
            listeners.emplace(other);
            other->invListeners.emplace(this);
        }

        void disconnect(Event *other) {
            listeners.erase(other);
            other->invListeners.erase(this);

            auto iter = typename owned_collection::iterator(other);
            if (iter >= ownedListeners.begin() && iter < ownedListeners.end()) {
                ownedListeners.erase(iter);
            }
        }

        void follow(Event *other) {
            following.emplace(other);
            other->followers.emplace(this);
        }

        void unfollow(Event *other) {
            following.erase(other);
            other->followers.erase(this);
        }

    private:
        std::optional<func_type> callback;
        owned_collection ownedListeners;
        std::set<Event *> listeners;
        std::set<Event *> invListeners;
        std::set<Event *> following;
        std::set<Event *> followers;

        void detachAll() {
            while (!invListeners.empty()) {
                (*invListeners.begin())->disconnect(this);
            }
            while (!followers.empty()) {
                (*followers.begin())->detachAll();
            }
            while (!following.empty()) {
                unfollow(*following.begin());
            }
        }

        void copyFrom(const Event &a) {
            for (const auto &listener : a.listeners) {
                auto iter = typename owned_collection::iterator(listener);
                if (iter >= a.ownedListeners.begin() && iter < a.ownedListeners.end()) {
                    listen(*iter);
                } else {
                    connect(listener);
                }
            }

            for (const auto &evt : a.following) {
                follow(evt);
            }
        }

        void moveFrom(Event &a) {
            callback = std::move(a.callback);
            ownedListeners = std::move(a.ownedListeners);
            listeners = std::move(a.listeners);
            invListeners = std::move(a.invListeners);
            following = std::move(a.following);
            followers = std::move(a.followers);

            for (const auto &listener : listeners) {
                listener->invListeners.erase(&a);
                listener->invListeners.emplace(this);
            }
            for (const auto &evt : following) {
                evt->followers.erase(&a);
                evt->followers.emplace(this);
            }
        }
    };

}
