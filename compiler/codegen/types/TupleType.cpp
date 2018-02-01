#include "TupleType.h"

#include "../Context.h"

using namespace MaximCodegen;

TupleType::TupleType(Context *context, std::vector<std::unique_ptr<Type>> types) : Type(context), _types(std::move(types)) {
    std::vector<llvm::Type*> realTypes;
    realTypes.reserve(_types.size());
    for (const auto &type : _types) {
        realTypes.push_back(type->llType());
    }
    _llType = llvm::StructType::create(context->llvm(), realTypes);
}
