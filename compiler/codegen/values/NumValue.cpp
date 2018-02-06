#include <iostream>
#include "NumValue.h"

#include "../Function.h"

using namespace MaximCodegen;

NumValue::NumValue(bool isConst, llvm::Value *value, const FormValue &form, Context *context, Function *function)
        : Value(isConst), _context(context) {
    _value = function->initBuilder().CreateAlloca(type(), nullptr, "num_val");

    auto cb = function->codeBuilder();
    cb.CreateStore(value, valuePtr(cb));
    cb.CreateStore(cb.CreateLoad(form.value(), "form_to_num_temp"), formPtr(cb));
}

NumValue::NumValue(bool isConst, llvm::Value *value, Context *context)
        : Value(isConst), _value(value), _context(context) {

}

std::unique_ptr<NumValue> NumValue::fromRegister(bool isConst, llvm::Value *value, Context *context,
                                                 Function *function) {
    auto numType = context->getStructType(Context::Type::NUM);
    auto alloc = function->initBuilder().CreateAlloca(numType, nullptr, "num_val");

    function->codeBuilder().CreateStore(value, alloc);

    return std::make_unique<NumValue>(isConst, alloc, context);
}

llvm::StructType *NumValue::type() const {
    return _context->getStructType(Context::Type::NUM);
}

llvm::Value *NumValue::valuePtr(llvm::IRBuilder<> &builder) const {
    return _context->getPtr(_value, 0, builder);
}

llvm::Value *NumValue::formPtr(llvm::IRBuilder<> &builder) const {
    return _context->getPtr(_value, 1, builder);
}

std::unique_ptr<Value> NumValue::clone() const {
    return std::make_unique<NumValue>(isConst(), _value, _context);
}
