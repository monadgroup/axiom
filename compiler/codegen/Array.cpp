#include "Array.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Array::Array(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _type(type), _get(get), _context(context) {
}

Array::Array(MaximContext *context, ArrayType *type, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _type(type), _context(context) {
    _get = allocaBuilder.CreateAlloca(type->get(), nullptr, "array");
}

std::unique_ptr<Array> Array::create(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Array>(context, type, get, startPos, endPos);
}

std::unique_ptr<Array> Array::create(MaximContext *context, ArrayType *type, Builder &allocaBuilder, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Array>(context, type, allocaBuilder, startPos, endPos);
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

std::unique_ptr<Value> Array::atIndex(size_t index, Builder &builder) {
    return atIndex(_context->constInt(32, index, false), builder);
}

std::unique_ptr<Value> Array::atIndex(llvm::Value *index, Builder &builder) {
    return _type->baseType()->createInstance(indexPtr(index, builder), startPos, endPos);
}

std::unique_ptr<Value> Array::withSource(SourcePos startPos, SourcePos endPos) const {
    return Array::create(_context, _type, _get, startPos, endPos);
}
