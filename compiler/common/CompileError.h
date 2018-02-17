#pragma once

#include "../SourcePos.h"

namespace MaximCommon {

    class CompileError {
    public:
        std::string message;
        SourcePos start;
        SourcePos end;

        CompileError(std::string message, SourcePos start, SourcePos end) : message(message), start(start), end(end) {}
    };

}
