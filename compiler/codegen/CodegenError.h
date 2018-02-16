#pragma once

namespace MaximCodegen {

    class CodegenError {
    public:
        std::string message;
        SourcePos start;
        SourcePos end;

        CodegenError(std::string message, SourcePos start, SourcePos end) : message(message), start(start), end(end) {}
    };

}
