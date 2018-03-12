#include "Array.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Array::Array(MaximContext *context, Storage values, Builder &builder, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _context(context) {
    assert(!values.empty());

    auto baseType = values[0]->type();
    for (size_t i = 1; i < values.size(); i++) {
        assert(values[i]->type() == baseType);
    }

    _type = context->getArrayType(baseType);

    _get = allocaBuilder.CreateAlloca(_type->get(), nullptr, "array");
    for (size_t i = 0; i < values.size(); i++) {
        setIndex(i, values[i].get(), builder);
    }
}

Array::Array(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _type(type), _get(get), _context(context) {
}

std::unique_ptr<Array> Array::create(MaximContext *context, Storage values, Builder &builder, Builder &allocaBuilder,
                                     SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Array>(context, std::move(values), builder, allocaBuilder, startPos, endPos);
}

std::unique_ptr<Array> Array::create(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Array>(context, type, get, startPos, endPos);
}

llvm::Value* Array::indexPtr(size_t index, Builder &builder) {
    return indexPtr(_context->constInt(32, index, false), builder);
}

llvm::Value* Array::indexPtr(llvm::Value *index, Builder &builder) {
    return builder.CreateGEP(_get, {
        _context->constInt(64, 0, false),
        index
    }, "arrayitem.ptr");
}

void Array::setIndex(size_t index, Value *val, Builder &builder) {
    setIndex(_context->constInt(32, index, false), val, builder);
}

void Array::setIndex(llvm::Value *index, Value *val, Builder &builder) {
    assert(val->type() == _type->baseType());

    auto ptr = indexPtr(index, builder);

    // todo: might be better to do a direct memcpy here
    auto loadedVal = builder.CreateLoad(val->get(), "arraystore");
    builder.CreateStore(loadedVal, ptr);
}

std::unique_ptr<Value> Array::atIndex(size_t index, Builder &builder, SourcePos startPos, SourcePos endPos) {
    auto ptr = indexPtr(index, builder);
    return _type->baseType()->createInstance(ptr, startPos, endPos);
}

std::unique_ptr<Value> Array::withSource(SourcePos startPos, SourcePos endPos) const {
    return Array::create(_context, _type, _get, startPos, endPos);
}

/*std::unique_ptr<Array> Array::withIndex(size_t index, std::unique_ptr<Value> val, Builder &builder,
                                        Builder &allocaBuilder, SourcePos startPos, SourcePos endPos) const {
    return withIndex(_context->constInt(32, index, false), std::move(val), builder, allocaBuilder, startPos, endPos);
}

std::unique_ptr<Array> Array::withIndex(llvm::Value *index, std::unique_ptr<Value> val, Builder &builder,
                                        Builder &allocaBuilder, SourcePos startPos, SourcePos endPos) const {
    auto newGet = allocaBuilder.CreateAlloca(_type->get(), nullptr, "array");
    auto newArray = Array::create(_context, _type, newGet, startPos, endPos);
    newArray->setIndex(index, val.get(), builder);
    return newArray;
}*/
