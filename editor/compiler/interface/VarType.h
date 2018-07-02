#pragma once

#include <cstddef>

#include "OwnedObject.h"

namespace MaximCompiler {

    class VarType : public OwnedObject {
    public:
        static VarType num();

        static VarType midi();

        static VarType tuple(VarType *types, size_t count);

        static VarType array(VarType subtype);

        VarType clone();

    private:
        explicit VarType(void *handle);
    };

}
