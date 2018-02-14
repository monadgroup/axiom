#include "TupleType.h"

#include <sstream>

#include "MaximContext.h"
#include "Tuple.h"

using namespace MaximCodegen;

TupleType::TupleType(MaximContext *context, std::vector<Type *> types, llvm::StructType *type) : _types(
    std::move(types)), _type(type), _context(context) {
}

std::string TupleType::name() const {
    std::stringstream s;
    s << "(";
    for (size_t i = 0; i < _types.size(); i++) {
        s << _types[i]->name();
        if (i < _types.size() - 1) s << ", ";
    }
    s << ")";
    return s.str();
}

std::unique_ptr<Value> TupleType::createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) {
    return Tuple::create(_context, this, val, startPos, endPos);
}
