#include "RawNumType.h"

#include "../Context.h"

using namespace MaximCodegen;

RawNumType::RawNumType(Context *context) : Type(context) {
    _leftType = llvm::Type::getFloatTy(context->llvm());
    _rightType = llvm::Type::getFloatTy(context->llvm());
    _llType = llvm::StructType::create(context->llvm(), std::array<llvm::Type *, 2> {
            _leftType,
            _rightType
    });
}
