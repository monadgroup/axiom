#pragma once

#include <vector>
#include "compiler/common/CompileError.h"

namespace MaximRuntime {

    class ErrorLog {
    public:
        std::vector<MaximCommon::CompileError> errors;
    };

}
