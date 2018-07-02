#pragma once

#include "OwnedObject.h"
#include "../model/Value.h"

namespace MaximCompiler {

    class ConstantValue : public OwnedObject {
    public:
        static ConstantValue num(AxiomModel::NumValue value);

        static ConstantValue tuple(ConstantValue *items, size_t count);

        ConstantValue clone();

    private:
        explicit ConstantValue(void *handle);
    };

}
