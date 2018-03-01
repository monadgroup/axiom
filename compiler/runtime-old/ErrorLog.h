#pragma once

#include <vector>
#include "../common/CompileError.h"

namespace MaximRuntime {

    class ErrorLog {
    public:
        std::vector<MaximCommon::CompileError> errors;
    };

}
