#include "FormValue.h"

#include "../Function.h"

using namespace MaximCodegen;

FormValue::FormValue(MaximAst::Form::Type formType, const ParamArr &params, Context *context, Function *function)
        : _context(context) {
    _value = function->initBuilder().CreateAlloca(type(), nullptr, "form_val_raw");

    std::array<llvm::Value *, Context::formParamCount> paramValues = {};
    switch (formType) {
        case MaximAst::Form::Type::LINEAR:
        case MaximAst::Form::Type::CONTROL:
            paramValues = {_context->getConstantFloat(0), _context->getConstantFloat(1)};
            break;
        case MaximAst::Form::Type::FREQUENCY:
            paramValues = {_context->getConstantFloat(20000), _context->getConstantFloat(0)};
            break;
        case MaximAst::Form::Type::NOTE:
        case MaximAst::Form::Type::DB:
        case MaximAst::Form::Type::Q:
        case MaximAst::Form::Type::RES:
        case MaximAst::Form::Type::SECONDS:
        case MaximAst::Form::Type::BEATS:
            paramValues = {_context->getConstantFloat(0), _context->getConstantFloat(0)};
            break;
    }

    auto cb = function->codeBuilder();
    cb.CreateStore(_context->getConstantInt(8, (unsigned int) formType, false), typePtr(cb));
    auto paramArrPtr = paramsPtr(cb);

    for (size_t i = 0; i < Context::formParamCount; i++) {
        auto setValue = params[i] ? params[i] : paramValues[i];
        cb.CreateStore(
                setValue,
                _context->getPtr(paramArrPtr, (unsigned int) i, cb)
        );
    }
}

FormValue::FormValue(llvm::Value *formType, llvm::Value *params, Context *context, Function *function) : _context(
        context) {
    _value = function->initBuilder().CreateAlloca(type(), nullptr, "form_val_copy");

    auto cb = function->codeBuilder();
    cb.CreateStore(formType, typePtr(cb));
    cb.CreateStore(params, paramsPtr(cb));
}

FormValue::FormValue(llvm::Value *value, Context *context) : _context(context), _value(value) {

}

llvm::Type *FormValue::type() const {
    return _context->getType(Context::Type::FORM);
}

llvm::Value *FormValue::typePtr(Builder &builder) const {
    return _context->getPtr(_value, 0, builder);
}

llvm::Value *FormValue::paramsPtr(Builder &builder) const {
    return _context->getPtr(_value, 1, builder);
}
