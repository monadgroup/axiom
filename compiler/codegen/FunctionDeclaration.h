#pragma once

#include <vector>
#include <memory>

#include "Parameter.h"

namespace llvm {
    class Type;
}

namespace MaximCodegen {

    class FunctionDeclaration {
    public:
        FunctionDeclaration(bool isPure, llvm::Type *returnType, std::vector<Parameter> parameters,
                 std::unique_ptr<Parameter> variadicParam);

        bool isPure() const { return _isPure; }

        llvm::Type *returnType() const { return _returnType; }

        std::vector<Parameter> const &parameters() const { return _parameters; }

        Parameter *variadicParam() const { return _variadicParam.get(); }

    private:
        bool _isPure;
        llvm::Type *_returnType;
        std::vector<Parameter> _parameters;
        std::unique_ptr<Parameter> _variadicParam;
    };

}
