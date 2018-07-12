#pragma once

#include "VarType.h"

namespace MaximCompiler {

    class RootRef {
    public:
        explicit RootRef(void *handle);

        void *get() const { return handle; }

        void addSocket(VarType vartype);

    private:
        void *handle;
    };
}
