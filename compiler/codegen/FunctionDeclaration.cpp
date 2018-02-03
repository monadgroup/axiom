#include "FunctionDeclaration.h"

using namespace MaximCodegen;

FunctionDeclaration::FunctionDeclaration(bool isPure, llvm::Type *returnType, std::vector<Parameter> parameters,
                   std::unique_ptr<Parameter> variadicParam)
        : _isPure(isPure), _returnType(returnType), _parameters(std::move(parameters)),
          _variadicParam(std::move(variadicParam)) {

}
