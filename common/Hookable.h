#pragma once

#include <set>

namespace AxiomCommon {

    class HookContext;

    class HookNotifiable {
    public:
        virtual void hookableDestroyed(HookContext *context) = 0;
    };

    class HookContext {
    public:
        HookContext() = default;

        HookContext(const HookContext &a);

        HookContext(HookContext &&a) noexcept;

        HookContext &operator=(const HookContext &a);

        HookContext &operator=(HookContext &&a) noexcept;

        ~HookContext();

        void addDestructHook(HookNotifiable *handle);

        void removeDestructHook(HookNotifiable *handle);

    private:
        std::set<HookNotifiable*> notifiables;

        void doDestruct();
    };

    class AbstractHookable {
    public:
        virtual HookContext *getContext() = 0;
    };

    class Hookable : public AbstractHookable {
    public:
        virtual ~Hookable() = default;

        HookContext *getContext() override { return &context; }

    private:
        HookContext context;
    };

}
