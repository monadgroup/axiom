#pragma once

#include <cstddef>

#include "OwnedObject.h"
#include "ConstantValue.h"

namespace MaximCompiler {

    class ValueGroupSource : public OwnedObject {
    public:
        static ValueGroupSource none();

        static ValueGroupSource socket(size_t index);

        static ValueGroupSource default_val(ConstantValue value);

        ValueGroupSource clone();

    private:
        explicit ValueGroupSource(void *handle);
    };

}
