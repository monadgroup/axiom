#include "MidiType.h"

#include "../Context.h"

using namespace MaximCodegen;

MidiType::MidiType(Context *context) : Type(context) {
    // todo: should these be 4 or 8 bits?
    _typeType = llvm::Type::getIntNTy(context->llvm(), 4);
    _channelType = llvm::Type::getIntNTy(context->llvm(), 4);

    _noteType = llvm::Type::getInt8Ty(context->llvm());
    _paramType = llvm::Type::getInt8Ty(context->llvm());
    _timeType = llvm::Type::getInt32Ty(context->llvm());
    _llType = llvm::StructType::create(context->llvm(), std::array<llvm::Type *, 5> {
            _typeType,
            _channelType,
            _noteType,
            _paramType,
            _timeType
    });
}
