#include "Array.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Array::Array(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _type(type), _get(get), _context(context) {
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

ArrayItem Array::atIndex(size_t index, Builder &builder) {
    return atIndex(_context->constInt(32, index, false), builder);
}

ArrayItem Array::atIndex(llvm::Value *index, Builder &builder) {
    return ArrayItem(
        indexPtr(index, builder),
        type()->itemType(),
        type()->baseType()
    );
}

std::unique_ptr<Value> Array::withSource(SourcePos startPos, SourcePos endPos) const {
    return Array::create(_context, _type, _get, startPos, endPos);
}

ArrayItem::ArrayItem(llvm::Value *get, llvm::Type *type, Type *itemType)
    : _get(get), _type(type), _itemType(itemType) {

}

llvm::Value* ArrayItem::enabledPtr(Builder &builder) const {
    return builder.CreateStructGEP(_type, _get, 0, _get->getName() + ".enabled.ptr");
}

llvm::Value* ArrayItem::enabled(Builder &builder) const {
    return builder.CreateLoad(enabledPtr(builder), _get->getName() + ".enabled");
}

std::unique_ptr<Value> ArrayItem::value(Builder &builder, SourcePos startPos, SourcePos endPos) const {
    auto ptr = builder.CreateStructGEP(_type, _get, 1, _get->getName() + ".value.ptr");
    return _itemType->createInstance(ptr, startPos, endPos);
}
