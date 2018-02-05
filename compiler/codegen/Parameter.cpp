#include "Parameter.h"

using namespace MaximCodegen;

Parameter::Parameter(bool isConst, llvm::Type *type, llvm::Constant *defaultValue)
        : _isConst(isConst), _type(type), _defaultValue(defaultValue) {

}
