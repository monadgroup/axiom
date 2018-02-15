#include "ArrayType.h"

#include "Array.h"

using namespace MaximCodegen;

ArrayType::ArrayType(MaximContext *context, Type *baseType, llvm::ArrayType *type)
    : _baseType(baseType), _type(type), _context(context) {

}

std::string ArrayType::name() const {
    return _baseType->name() + "[]";
}

std::unique_ptr<Value> ArrayType::createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) {
    return Array::create(_context, this, val, startPos, endPos);
}
