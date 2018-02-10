#pragma once

#include <memory>

#include "Value.h"

namespace MaximCodegen {

    class Operator {
    public:
        std::unique_ptr<Value> call(std::unique_ptr<Value> left, std::unique_ptr<Value> right);
    };

}
