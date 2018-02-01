#include "Function.h"

#include "types/TupleType.h"
#include "values/Value.h"

using namespace MaximCodegen;

Function::Function(bool isPure, std::unique_ptr<TupleType> returnType, std::vector<Parameter> parameters,
                   std::unique_ptr<Parameter> variadicParam)
        : _isPure(isPure), _returnType(std::move(returnType)), _parameters(std::move(parameters)),
          _variadicParam(std::move(variadicParam)) {

}
