#include "NumValue.h"

#include "../Function.h"

using namespace MaximCodegen;

NumValue::NumValue(bool isConst, llvm::Value *value, const FormValue &form, Context *context, Function *function)
        : Value(isConst), _context(context) {
    _value = function->initBuilder().CreateAlloca(type());

    auto cb = function->codeBuilder();
    cb.CreateStore(value, valuePtr(cb));
    cb.CreateStore(form.value(), formPtr(cb));
}

NumValue::NumValue(bool isConst, llvm::Value *value, Context *context)
        : Value(isConst), _value(value), _context(context) {

}

llvm::StructType* NumValue::type() const {
    return _context->getStructType(Context::Type::NUM);
}

llvm::Value* NumValue::valuePtr(llvm::IRBuilder<> &builder) const {
    return _context->getStructParamPtr(_value, type(), 0, builder);
}

llvm::Value* NumValue::formPtr(llvm::IRBuilder<> &builder) const {
    return _context->getStructParamPtr(_value, type(), 1, builder);
}

std::unique_ptr<Value> NumValue::clone() const {
    return std::make_unique<NumValue>(isConst(), _value, _context);
}
