#include "NumType.h"

#include "../Context.h"
#include "RawNumType.h"
#include "FormType.h"

using namespace MaximCodegen;

NumType::NumType(Context *context, Form *form) : Type(context) {
    _numType = std::make_unique<RawNumType>(context);
    _formType = std::make_unique<FormType>(context, form);
    _llType = llvm::StructType::create(context->llvm(), std::array<llvm::Type *, 2> {
            _numType->llType(),
            _formType->llType()
    });
}
