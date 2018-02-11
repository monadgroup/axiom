#include "Midi.h"

#include <llvm/IR/Constants.h>

#include "MaximContext.h"

using namespace MaximCodegen;

Midi::Midi(MaximContext *context, MaximCommon::MidiEventType event, uint8_t channel, uint8_t note, uint8_t param,
           SourcePos startPos, SourcePos endPos) : Value(startPos, endPos), _context(context) {
    _get = llvm::ConstantStruct::get(type()->get(), {
        llvm::ConstantInt::get(type()->eventType(), (uint64_t) event, false),
        llvm::ConstantInt::get(type()->channelType(), (uint64_t) channel, false),
        llvm::ConstantInt::get(type()->noteType(), (uint64_t) note, false),
        llvm::ConstantInt::get(type()->paramType(), (uint64_t) param, false)
    });
}

Midi::Midi(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context) {
}

std::unique_ptr<Midi> Midi::create(MaximContext *context, MaximCommon::MidiEventType event, uint8_t channel,
                                   uint8_t note, uint8_t param, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Midi>(context, event, channel, note, param, startPos, endPos);
}

std::unique_ptr<Midi> Midi::create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Midi>(context, get, startPos, endPos);
}

llvm::Value *Midi::event(Builder &builder) const {
    return builder.CreateExtractValue(_get, {0}, "midi.event");
}

llvm::Value *Midi::channel(Builder &builder) const {
    return builder.CreateExtractValue(_get, {1}, "midi.channel");
}

llvm::Value *Midi::note(Builder &builder) const {
    return builder.CreateExtractValue(_get, {2}, "midi.note");
}

llvm::Value *Midi::param(Builder &builder) const {
    return builder.CreateExtractValue(_get, {3}, "midi.param");
}

MidiType *Midi::type() const {
    return _context->midiType();
}
