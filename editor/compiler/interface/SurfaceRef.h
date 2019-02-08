#pragma once

#include "ControlInitializer.h"
#include "NodeRef.h"
#include "ValueGroupSource.h"
#include "VarType.h"

namespace MaximCompiler {

    class SurfaceRef {
    public:
        explicit SurfaceRef(void *handle);

        void *get() const { return handle; }

        void addValueGroup(VarType vartype, ValueGroupSource source);

        NodeRef addCustomNode(uint64_t blockId, size_t controlInitializerCount, ControlInitializer *initializers);

        NodeRef addGroupNode(uint64_t surfaceId);

    private:
        void *handle;
    };
}
