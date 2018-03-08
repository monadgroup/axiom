#include "MidiType.h"

#include "MaximContext.h"
#include "Midi.h"

using namespace MaximCodegen;

MidiType::MidiType(MaximContext *context) : _context(context) {
    _typeType = llvm::Type::getIntNTy(context->llvm(), 4);
    _channelType = llvm::Type::getIntNTy(context->llvm(), 4);
    _noteType = llvm::Type::getInt8Ty(context->llvm());
    _paramType = llvm::Type::getInt8Ty(context->llvm());
    _eventType = llvm::StructType::create(context->llvm(), {
        _typeType, _channelType, _noteType, _paramType
    }, "struct.midievent");

    _arrayType = llvm::ArrayType::get(_eventType, maxEvents);
    _countType = llvm::Type::getInt8Ty(context->llvm());
    _type = llvm::StructType::create(context->llvm(), {
        _countType, _arrayType
    }, "struct.midi");

    _eventLayout = context->dataLayout().getStructLayout(_eventType);
    _layout = context->dataLayout().getStructLayout(_type);
}

std::unique_ptr<Value> MidiType::createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) {
    return Midi::create(_context, val, startPos, endPos);
}
