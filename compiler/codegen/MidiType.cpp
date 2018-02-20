#include "MidiType.h"

#include "MaximContext.h"
#include "Midi.h"

using namespace MaximCodegen;

MidiType::MidiType(MaximContext *context) : _context(context) {
    _eventType = llvm::Type::getIntNTy(context->llvm(), 4);
    _channelType = llvm::Type::getIntNTy(context->llvm(), 4);
    _noteType = llvm::Type::getInt8Ty(context->llvm());
    _paramType = llvm::Type::getInt8Ty(context->llvm());
    _type = llvm::StructType::create(context->llvm(), {
        _eventType, _channelType, _noteType, _paramType
    }, "struct.midi");
    _layout = context->dataLayout().getStructLayout(_type);
}

std::unique_ptr<Value> MidiType::createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) {
    return Midi::create(_context, val, startPos, endPos);
}
