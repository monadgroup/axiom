#include "ArrayType.h"

#include "Array.h"
#include "MaximContext.h"

using namespace MaximCodegen;

ArrayType::ArrayType(MaximContext *context, Type *baseType)
    : _baseType(baseType), _context(context) {
    _itemType = baseType->get();
    _type = llvm::ArrayType::get(_itemType, arraySize);
}

std::string ArrayType::name() const {
    return _baseType->name() + "[]";
}

std::unique_ptr<Value> ArrayType::createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) {
    return Array::create(_context, this, val, startPos, endPos);
}
