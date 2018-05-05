#pragma once

#include <functional>
#include <set>

namespace AxiomModel {

    template<class A>
    class Promise {
    public:
        using func_type = std::function<void(const A &)>;

        Promise() = default;

        ~Promise() {
            doDestruct();
        }

        Promise(Promise &a) noexcept {
            copyFrom(a);
        }

        Promise(Promise &&a) noexcept {
            moveFrom(a);
        }

        Promise &operator=(Promise &a) noexcept {
            doDestruct();
            copies.clear();
            copyFrom(a);
            return *this;
        }

        Promise &operator=(Promise &&a) noexcept {
            doDestruct();
            copies.clear();
            moveFrom(a);
            return *this;
        }

        const std::optional<A> &value() const { return _value; }

        void then(func_type func) {
            if (_value) {
                func(*_value);
            } else {
                callbacks.push_back(func);
            }
        }

        void resolve(const A &val) {
            if (_value) return;

            if (copyOf) copyOf->resolve(val);
            else doResolve(val);
        }

    private:
        std::vector<func_type> callbacks;
        std::optional<A> _value;

        Promise *copyOf = nullptr;
        std::set<Promise *> copies;

        void doDestruct() {
            if (copyOf) {
                copyOf->copies.erase(this);
            }

            for (const auto &copied : copies) {
                copied->copyOf = nullptr;
            }
        }

        void copyFrom(Promise &a) {
            callbacks = a.callbacks;
            _value = a._value;
            copyOf = &a;
            a.copies.emplace(this);
        }

        void moveFrom(Promise &a) {
            callbacks = std::move(a.callbacks);
            _value = std::move(a._value);

            copyOf = a.copyOf;
            if (copyOf) {
                copyOf->copies.erase(&a);
                copyOf->copies.emplace(this);
            }
        }

        void doResolve(const A &val) {
            _value = val;

            for (const auto &cb : callbacks) {
                cb(*_value);
            }

            for (const auto &copy : copies) {
                copy->doResolve(val);
            }
        }
    };

}
