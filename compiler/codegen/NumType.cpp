#include "NumType.h"

#include "MaximContext.h"
#include "Num.h"

using namespace MaximCodegen;

NumType::NumType(MaximContext *context) : _context(context) {
    _vecType = llvm::VectorType::get(llvm::Type::getFloatTy(context->llvm()), 2);
    _formType = llvm::Type::getInt8Ty(context->llvm());
    _activeType = llvm::Type::getInt8Ty(context->llvm());
    _type = llvm::StructType::create(context->llvm(), {_activeType, _vecType, _formType}, "struct.num");
    _layout = context->dataLayout().getStructLayout(_type);
}

std::unique_ptr<Value> NumType::createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) {
    return Num::create(_context, val, startPos, endPos);
}
