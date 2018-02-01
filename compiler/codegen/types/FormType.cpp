#include "FormType.h"

#include "../Context.h"
#include "../Form.h"

using namespace MaximCodegen;

FormType::FormType(Context *context, Form *form) : Type(context), _form(form) {
    _typeType = llvm::Type::getInt8Ty(context->llvm());
    _parametersType = form->type();
    _llType = llvm::StructType::create(context->llvm(), std::array<llvm::Type *, 2> {
            _typeType,
            _parametersType
    });
}
