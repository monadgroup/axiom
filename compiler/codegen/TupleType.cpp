#include "TupleType.h"

#include "MaximContext.h"
#include "Tuple.h"

using namespace MaximCodegen;

TupleType::TupleType(MaximContext *context, std::vector<Type*> types, llvm::StructType *type) : _types(std::move(types)), _type(type), _context(context) {
}

std::unique_ptr<Value> TupleType::createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) {
    return Tuple::create(_context, this, val, startPos, endPos);
}
