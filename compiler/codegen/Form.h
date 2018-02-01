#pragma once

#include <vector>

#include "Parameter.h"

namespace llvm {
    class StructType;
}

namespace MaximCodegen {

    class Context;

    class Form {
    public:
        explicit Form(Context *context, std::vector<Parameter> parameters);

        std::vector<Parameter> const &parameters() const { return _parameters; }

        llvm::StructType *type() const { return _type; }

    private:
        std::vector<Parameter> _parameters;
        llvm::StructType *_type;
    };

}
