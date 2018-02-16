#pragma once

#include "../SourcePos.h"

namespace MaximRuntime {

    class RuntimeError {
    public:
        std::string message;
        SourcePos start;
        SourcePos end;

        RuntimeError(std::string message, SourcePos start, SourcePos end) : message(message), start(start), end(end) {}
    };

}
