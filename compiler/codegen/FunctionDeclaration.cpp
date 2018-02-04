#include "FunctionDeclaration.h"

#include <llvm/IR/Function.h>

using namespace MaximCodegen;

FunctionDeclaration::FunctionDeclaration(bool isPure, llvm::Type *returnType, std::vector<Parameter> parameters,
                                         std::unique_ptr<Parameter> variadicParam)
        : _isPure(isPure), _returnType(returnType), _parameters(parameters), _variadicParam(std::move(variadicParam)) {

    std::vector<llvm::Type *> paramTypes;
    paramTypes.reserve(_parameters.size());
    for (const auto &param : _parameters) {
        paramTypes.push_back(param.type());
    }

    _type = llvm::FunctionType::get(_returnType, paramTypes, (bool) _variadicParam);
}
