#pragma once

#include <memory>

#include "Hookable.h"

namespace AxiomCommon {

    class SharedHookable : public AbstractHookable {
    public:
        SharedHookable();

        virtual ~SharedHookable() = default;

        HookContext *getContext() { return impl.get(); }

    private:
        std::shared_ptr<HookContext> impl;
    };
}
