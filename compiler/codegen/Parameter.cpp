#include "Parameter.h"

#include "types/Type.h"
#include "values/Value.h"

using namespace MaximCodegen;

Parameter::Parameter(bool isConst, std::unique_ptr<Type> type, std::unique_ptr<Value> defaultValue)
        : _isConst(isConst), _type(std::move(type)), _defaultValue(std::move(defaultValue)) {

}
