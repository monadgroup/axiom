#include "Array.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Array::Array(MaximContext *context, Storage values, Builder &builder, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _context(context) {
    assert(!values.empty());

    auto baseType = values[0]->type();
    for (size_t i = 1; i < values.size(); i++) {
        assert(values[i]->type() == baseType);
    }

    _type = context->getArrayType(baseType);

    _get = llvm::UndefValue::get(_type->get());
    for (size_t i = 0; i < values.size(); i++) {
        _get = builder.CreateInsertValue(_get, values[i]->get(), {(unsigned int) i}, "array");
    }
}

Array::Array(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _type(type), _get(get), _context(context) {
}

std::unique_ptr<Array> Array::create(MaximContext *context, Storage values, Builder &builder, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Array>(context, std::move(values), builder, startPos, endPos);
}

std::unique_ptr<Array> Array::create(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Array>(context, type, get, startPos, endPos);
}

std::unique_ptr<Value> Array::atIndex(size_t index, Builder &builder, SourcePos startPos, SourcePos endPos) const {
    assert(index < ArrayType::arraySize);
    auto extracted = builder.CreateExtractValue(_get, {(unsigned int) index}, "array.extract");
    return _type->baseType()->createInstance(extracted, startPos, endPos);
}

std::unique_ptr<Value> Array::withSource(SourcePos startPos, SourcePos endPos) const {
    return Array::create(_context, _type, _get, startPos, endPos);
}

std::unique_ptr<Array> Array::withIndex(size_t index, std::unique_ptr<Value> val, Builder &builder, SourcePos startPos,
                                        SourcePos endPos) const {
    assert(index < ArrayType::arraySize);
    auto inserted = builder.CreateInsertValue(_get, val->get(), {(unsigned int) index}, "array.inserted");
    return Array::create(_context, _type, inserted, startPos, endPos);
}
