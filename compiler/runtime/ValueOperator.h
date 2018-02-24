#pragma once

#include "../common/FormType.h"

namespace MaximCodegen {
    class MaximContext;
}

namespace MaximRuntime {

    struct NumValue {
        float left = 0;
        float right = 0;
        MaximCommon::FormType form = MaximCommon::FormType::LINEAR;
        bool active = false;

        bool operator==(const NumValue &other) const {
            return left == other.left && right == other.right && form == other.form && active == other.active;
        }

        bool operator!=(const NumValue &other) const {
            return !(*this == other);
        }

        NumValue withLR(float l, float r) const {
            return {l, r, form, active};
        }

        NumValue withL(float l) const {
            return {l, right, form, active};
        }

        NumValue withR(float r) const {
            return {left, r, form, active};
        }
    };

    class ValueOperator {
    public:
        explicit ValueOperator(MaximCodegen::MaximContext *context);

        NumValue readNum(void *ptr);

        void writeNum(void *ptr, NumValue value);

    private:
        MaximCodegen::MaximContext *_context;

        uint64_t numValOffset;
        uint64_t numFormOffset;
        uint64_t numActiveOffset;
    };

}
