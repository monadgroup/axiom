#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "Parameter.h"

namespace llvm {
    class Type;
    class Module;
    class FunctionType;
}

namespace MaximCodegen {

    class Context;

    class FunctionDeclaration {
    public:
        FunctionDeclaration(bool isPure, llvm::Type *returnType, std::vector<Parameter> parameters,
                            std::unique_ptr<Parameter> variadicParam = nullptr, llvm::Constant *storage = nullptr);

        bool isPure() const { return _isPure; }

        llvm::Type *returnType() const { return _returnType; }

        llvm::Type *vaType() const { return _vaType; }

        std::vector<Parameter> const &parameters() const { return _parameters; }

        Parameter *variadicParam() const { return _variadicParam.get(); }

        llvm::FunctionType *type() const { return _type; }

        llvm::Constant *storage() const { return _storage; }

        size_t minParamCount() const { return _minParamCount; }

        int maxParamCount() const { return _maxParamCount; }

        size_t requiredParamCount() const { return _requiredParamCount; }

        size_t optionalParamCount() const { return _optionalParamCount; }

        Parameter *getParameter(size_t index);

    private:
        bool _isPure;
        llvm::Type *_returnType;
        llvm::Type *_vaType = nullptr;
        std::vector<Parameter> _parameters;
        std::unique_ptr<Parameter> _variadicParam;
        llvm::FunctionType *_type;
        llvm::Constant *_storage;

        size_t _minParamCount;
        int _maxParamCount;
        size_t _requiredParamCount;
        size_t _optionalParamCount;
    };

}
