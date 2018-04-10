#include "Tuple.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Tuple::Tuple(MaximContext *context, Storage values, Builder &builder, Builder &allocaBuilder, SourcePos startPos,
             SourcePos endPos)
    : Value(startPos, endPos), _context(context) {

    std::vector<Type *> types;
    types.reserve(values.size());
    for (const auto &val : values) {
        types.push_back(val->type());
    }
    _type = context->getTupleType(types);

    _get = allocaBuilder.CreateAlloca(_type->get(), nullptr, "tuple");
    for (size_t i = 0; i < values.size(); i++) {
        setIndex(i, values[i].get(), builder);
    }
}

Tuple::Tuple(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _type(type), _get(get), _context(context) {
}

std::unique_ptr<Tuple> Tuple::create(MaximContext *context, Storage values, Builder &builder, Builder &allocaBuilder,
                                     SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Tuple>(context, std::move(values), builder, allocaBuilder, startPos, endPos);
}

std::unique_ptr<Tuple> Tuple::create(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos,
                                     SourcePos endPos) {
    return std::make_unique<Tuple>(context, type, get, startPos, endPos);
}

llvm::Value *Tuple::indexPtr(size_t index, Builder &builder) const {
    return builder.CreateStructGEP(_type->get(), _get, (unsigned int) index, "tupleitem.ptr");
}

void Tuple::setIndex(size_t index, Value *val, Builder &builder) const {
    assert(index < _type->types().size());
    assert(_type->types()[index] == val->type());

    auto ptr = indexPtr(index, builder);
    _context->copyPtr(builder, val->get(), ptr);
}

std::unique_ptr<Value> Tuple::atIndex(size_t index, Builder &builder, SourcePos startPos, SourcePos endPos) const {
    assert(index < _type->types().size());

    auto ptr = indexPtr(index, builder);
    return _type->types()[index]->createInstance(ptr, startPos, endPos);
}

std::unique_ptr<Value> Tuple::withSource(SourcePos startPos, SourcePos endPos) const {
    return Tuple::create(_context, _type, _get, startPos, endPos);
}
