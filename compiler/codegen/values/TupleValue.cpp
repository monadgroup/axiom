#include "TupleValue.h"

#include "../Context.h"
#include "../Function.h"

using namespace MaximCodegen;

TupleValue::TupleValue(bool isConst, const std::vector<llvm::Value *> &values, Context *context, Function *function)
        : Value(isConst), _context(context) {
    std::vector<llvm::Type *> tupleTypes;
    tupleTypes.reserve(values.size());
    for (const auto &value : values) {
        tupleTypes.push_back(value->getType());
    }

    _type = llvm::StructType::get(_context->llvm(), tupleTypes);
    _value = function->initBuilder().CreateAlloca(_type);
    for (size_t i = 0; i < values.size(); i++) {
        function->codeBuilder().CreateStore(
                values[i],
                _context->getStructParamPtr(_value, _type, (unsigned int) i, function->codeBuilder())
        );
    }
}

TupleValue::TupleValue(bool isConst, llvm::Value *value, Context *context)
        : Value(isConst), _value(value), _context(context) {
    assert(value->getType()->isStructTy());
    _type = (llvm::StructType*) value->getType();
}

llvm::Value* TupleValue::itemPtr(unsigned int index, llvm::IRBuilder<> &builder) const {
    return _context->getStructParamPtr(_value, _type, index, builder);
}

std::unique_ptr<Value> TupleValue::clone() const {
    return std::make_unique<TupleValue>(isConst(), _value, _context);
}
