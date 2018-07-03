#pragma once

#include <functional>
#include <set>

#include "Event.h"

namespace AxiomCommon {

    template<class A>
    class Promise {
    public:
        using func_type = typename Event<A &>::func_type;

        Promise() : _value(std::make_shared<std::optional<A>>(std::optional<A>())) {}

        std::optional<A> &value() {
            return *_value;
        }

        const std::optional<A> &value() const {
            return *_value;
        }

        void then(func_type func) {
            if (*_value) {
                func(**_value);
            } else {
                event.connect(std::move(func));
            }
        }

        void then(Hookable *hook, func_type func) {
            if (*_value) {
                func(**_value);
            } else {
                event.connect(hook, std::move(func));
            }
        }

        void resolve(A val) {
            if (*_value) return;

            *_value = std::move(val);
            event.trigger(**_value);
            event.detach();
        }

    private:
        Event<A &> event;
        std::shared_ptr<std::optional<A>> _value;
    };
}
