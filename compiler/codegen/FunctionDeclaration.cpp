#include "FunctionDeclaration.h"

#include <llvm/IR/Function.h>

using namespace MaximCodegen;

FunctionDeclaration::FunctionDeclaration(bool isPure, llvm::Type *returnType, std::vector<Parameter> parameters,
                                         std::unique_ptr<Parameter> variadicParam)
        : _isPure(isPure), _returnType(returnType), _parameters(std::move(parameters)), _variadicParam(std::move(variadicParam)) {
    _requiredParamCount = parameters.size();
    _optionalParamCount = 0;
    _minParamCount = _requiredParamCount + (_variadicParam ? 1 : 0);
    _maxParamCount = _variadicParam ? -1 : (int) _parameters.size();

    std::vector<llvm::Type *> paramTypes;
    paramTypes.reserve(_minParamCount);

    for (const auto &param : _parameters) {
        paramTypes.push_back(param.type());
        if (param.defaultValue()) {
            _requiredParamCount--;
            _optionalParamCount++;
            _minParamCount--;
        }
    }

    if (_variadicParam) {
        _vaType = llvm::StructType::get(returnType->getContext(), std::array<llvm::Type*, 2>{
                llvm::Type::getInt8Ty(returnType->getContext()),
                llvm::PointerType::get(_variadicParam->type(), 0)
        });
        paramTypes.push_back(_vaType);
    }

    _type = llvm::FunctionType::get(_returnType, paramTypes, false);
}

Parameter* FunctionDeclaration::getParameter(size_t index) {
    assert(index >= 0);

    if (index >= _parameters.size()) {
        assert(_variadicParam);
        return _variadicParam.get();
    } else {
        return &_parameters[index];
    }
}
