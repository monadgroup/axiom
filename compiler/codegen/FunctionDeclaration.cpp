#include "FunctionDeclaration.h"

#include <llvm/IR/Function.h>

using namespace MaximCodegen;

FunctionDeclaration::FunctionDeclaration(bool isPure, llvm::Type *returnType, std::vector<Parameter> parameters,
                                         std::unique_ptr<Parameter> variadicParam)
        : _isPure(isPure), _returnType(returnType), _parameters(std::move(parameters)), _variadicParam(std::move(variadicParam)) {

    _minParamCount = _parameters.size() + (_variadicParam ? 1 : 0);
    _maxParamCount = _variadicParam ? -1 : (int) _parameters.size();

    std::vector<llvm::Type *> paramTypes;
    paramTypes.reserve(_parameters.size() + 1);

    // always add a parameter for number of parameters passed (used for variadic funcs)
    paramTypes.push_back(llvm::Type::getInt8Ty(returnType->getContext()));

    for (const auto &param : _parameters) {
        paramTypes.push_back(param.type());
        if (param.defaultValue()) _minParamCount--;
    }

    _type = llvm::FunctionType::get(_returnType, paramTypes, (bool) _variadicParam);
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
