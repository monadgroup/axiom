#include "Tuple.h"

#include <llvm/IR/Constants.h>

#include "MaximContext.h"

using namespace MaximCodegen;

Tuple::Tuple(MaximContext *context, Storage values, Builder &builder, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _context(context) {

    std::vector<Type *> types;
    types.reserve(values.size());
    for (const auto &val : values) {
        types.push_back(val->type());
    }
    _type = context->getTupleType(types);

    _get = llvm::UndefValue::get(_type->get());
    for (size_t i = 0; i < values.size(); i++) {
        _get = builder.CreateInsertValue(_get, values[i]->get(), {(unsigned int) i}, "tuple");
    }
}

Tuple::Tuple(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context) {
}

std::unique_ptr<Tuple> Tuple::create(MaximContext *context, Storage values, Builder &builder, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Tuple>(context, std::move(values), builder, startPos, endPos);
}

std::unique_ptr<Tuple> Tuple::create(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Tuple>(context, type, get, startPos, endPos);
}

std::unique_ptr<Value> Tuple::atIndex(size_t index, Builder &builder, SourcePos startPos, SourcePos endPos) const {
    assert(index < _type->types().size());
    auto extracted = builder.CreateExtractValue(_get, {(unsigned int) index}, "tuple.extract");
    return _type->types()[index]->createInstance(extracted, startPos, endPos);
}

std::unique_ptr<Value> Tuple::withSource(SourcePos startPos, SourcePos endPos) const {
    return Tuple::create(_context, _type, _get, startPos, endPos);
}

std::unique_ptr<Tuple> Tuple::withIndex(size_t index, std::unique_ptr<Value> val, Builder &builder, SourcePos startPos,
                                        SourcePos endPos) const {
    assert(index < _type->types().size());
    assert(_type->types()[index] == val->type());

    auto inserted = builder.CreateInsertValue(_get, val->get(), {(unsigned int) index}, "tuple.insert");
    return Tuple::create(_context, _type, inserted, startPos, endPos);
}
