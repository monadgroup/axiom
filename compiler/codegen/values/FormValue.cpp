#include "FormValue.h"

#include "../Function.h"

using namespace MaximCodegen;

FormValue::FormValue(MaximAst::Form::Type formType, const ParamArr &params, Context *context, Function *function)
        : _context(context) {
    _value = function->initBuilder().CreateAlloca(type());

    std::array<llvm::Value *, Context::formParamCount> paramValues = {};
    switch (formType) {
        case MaximAst::Form::Type::LINEAR:
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

    auto paramsType = type()->getStructElementType(1);
    auto paramsVal = function->initBuilder().CreateAlloca(paramsType);

    auto cb = function->codeBuilder();
    for (size_t i = 0; i < Context::formParamCount; i++) {
        auto setValue = params[i] ? params[i] : paramValues[i];
        cb.CreateStore(
                setValue,
                _context->getStructParamPtr(paramsVal, paramsType, (unsigned int) i, cb)
        );
    }

    cb.CreateStore(_context->getConstantInt(8, (unsigned int) formType, false), typePtr(cb));
    cb.CreateStore(paramsVal, paramsPtr(cb));
}

FormValue::FormValue(llvm::Value *formType, llvm::Value *params, Context *context, Function *function) : _context(
        context) {
    _value = function->initBuilder().CreateAlloca(type());

    auto cb = function->codeBuilder();
    cb.CreateStore(formType, typePtr(cb));
    cb.CreateStore(params, paramsPtr(cb));
}

FormValue::FormValue(llvm::Value *value, Context *context) : _context(context), _value(value) {

}

llvm::Type *FormValue::type() const {
    return _context->getType(Context::Type::FORM);
}

llvm::Value *FormValue::typePtr(llvm::IRBuilder<> &builder) const {
    return _context->getStructParamPtr(_value, type(), 0, builder);
}

llvm::Value *FormValue::paramsPtr(llvm::IRBuilder<> &builder) const {
    return _context->getStructParamPtr(_value, type(), 1, builder);
}
