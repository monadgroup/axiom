#pragma once

#include <set>

namespace AxiomModel {

    class Hookable;

    class HookNotifiable {
    public:
        virtual void hookableDestroyed(Hookable *hookable) = 0;
    };

    class Hookable {
    public:
        Hookable() = default;

        Hookable(const Hookable &a);

        Hookable(Hookable &&a) noexcept;

        Hookable &operator=(const Hookable &a);

        Hookable &operator=(Hookable &&a) noexcept;

        virtual ~Hookable();

        void addDestructHook(HookNotifiable *handle);

        void removeDestructHook(HookNotifiable *handle);

    private:
        std::set<HookNotifiable*> notifiables;

        void doDestruct();
    };

}
