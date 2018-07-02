#pragma once

#include "VarType.h"
#include "ValueGroupSource.h"
#include "NodeRef.h"

namespace MaximCompiler {

    class SurfaceRef {
    public:
        explicit SurfaceRef(void *handle);

        void *get() const { return handle; }

        void addValueGroup(VarType vartype, ValueGroupSource source);

        NodeRef addCustomNode(uint64_t blockId);

        NodeRef addGroupNode(uint64_t surfaceId);

    private:
        void *handle;
    };

}
