#pragma once

#include <vector>
#include "../parser/ParseError.h"
#include "../codegen/CodegenError.h"

namespace MaximRuntime {

    class ErrorLog {
    public:
        std::vector<MaximParser::ParseError> parseErrors;
        std::vector<MaximCodegen::CodegenError> codegenErrors;
    };

}
