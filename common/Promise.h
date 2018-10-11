#pragma once

#include "Event.h"

namespace AxiomCommon {

    template<class V>
    class Promise {
    public:
        using Value = V;
        using FuncType = typename Event<Value &>::FuncType;

        Promise() = default;

        Promise(Promise &&a) noexcept : event(std::move(a.event)), _value(std::move(a._value)) {}

        std::optional<Value> &value() { return _value; }

        const std::optional<Value> &value() const { return _value; }

        void then(FuncType func) {
            if (_value) {
                func(*_value);
            } else {
                event.connect(std::move(func));
            }
        }

        void then(TrackedObject *obj, FuncType func) {
            if (_value) {
                func(*_value);
            } else {
                event.connect(obj, std::move(func));
            }
        }

        void resolve(Value val) {
            if (_value) return;

            _value = std::move(val);
            event(*_value);
            event.disconnectAll();
        }

    private:
        Event<Value &> event;
        std::optional<Value> _value;
    };
}
