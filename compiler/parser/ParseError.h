#pragma once

#include "../SourcePos.h"

namespace MaximParser {

    class ParseError {
    public:
        std::string message;
        SourcePos start;
        SourcePos end;

        ParseError(std::string message, SourcePos start, SourcePos end) : message(message), start(start), end(end) { }
    };

}
