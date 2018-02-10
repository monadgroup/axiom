#include "MaximContext.h"

#include <llvm/IR/DerivedTypes.h>

using namespace MaximCodegen;

MaximContext::MaximContext() {
    _numVecType = llvm::VectorType::get(llvm::Type::getFloatTy(_llvm), 2);
    _numFormType = llvm::Type::getInt8Ty(_llvm);
    _numActiveType = llvm::Type::getInt1Ty(_llvm);
    _numType = llvm::StructType::create(_llvm, {_numVecType, _numFormType, _numActiveType}, "struct.num");

    _midiEventType = llvm::Type::getIntNTy(_llvm, 4);
    _midiChannelType = llvm::Type::getIntNTy(_llvm, 4);
    _midiNoteType = llvm::Type::getInt8Ty(_llvm);
    _midiParamType = llvm::Type::getInt8Ty(_llvm);
    _midiType = llvm::StructType::create(_llvm, {
        _midiEventType, _midiChannelType, _midiNoteType, _midiParamType
    }, "struct.midi");
}
