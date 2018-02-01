#include "Form.h"

#include <llvm/IR/DerivedTypes.h>

#include "Context.h"
#include "types/Type.h"
#include "values/Value.h"

using namespace MaximCodegen;

Form::Form(Context *context, std::vector<Parameter> parameters) : _parameters(std::move(parameters)) {
    std::vector<llvm::Type *> realTypes;
    realTypes.reserve(_parameters.size());
    for (const auto &param : _parameters) {
        realTypes.push_back(param.type()->llType());
    }
    _type = llvm::StructType::create(context->llvm(), realTypes);
}
