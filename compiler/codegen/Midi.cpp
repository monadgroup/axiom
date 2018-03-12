#include "Midi.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Midi::Midi(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context ){

}

std::unique_ptr<Midi> Midi::create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Midi>(context, get, startPos, endPos);
}

llvm::Value* Midi::countPtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 0, _get->getName() + ".count.ptr");
}

llvm::Value* Midi::eventsPtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 1, _get->getName() + ".events.ptr");
}

llvm::Value* Midi::eventPtr(Builder &builder, size_t index) const {
    return eventPtr(builder, _context->constInt(32, index, false));
}

llvm::Value* Midi::eventPtr(Builder &builder, llvm::Value *index) const {
    return builder.CreateGEP(
        _get,
        {
            _context->constInt(64, 0, false),
            _context->constInt(32, 1, false),
            index
        },
        _get->getName() + ".event.ptr"
    );
}

llvm::Value* Midi::count(Builder &builder) const {
    return builder.CreateLoad(countPtr(builder), _get->getName() + ".count");
}

MidiEvent Midi::eventAt(Builder &builder, size_t index) const {
    return eventAt(builder, _context->constInt(32, index, false));
}

MidiEvent Midi::eventAt(Builder &builder, llvm::Value *index) const {
    return MidiEvent(eventPtr(builder, index), type()->eventType());
}

void Midi::setCount(Builder &builder, uint64_t count) const {
    setCount(builder, llvm::ConstantInt::get(type()->countType(), count, false));
}

void Midi::setCount(Builder &builder, llvm::Value *count) const {
    builder.CreateStore(count, countPtr(builder));
}

std::unique_ptr<Value> Midi::withSource(SourcePos startPos, SourcePos endPos) const {
    return create(_context, _get, startPos, endPos);
}

MidiType* Midi::type() const {
    return _context->midiType();
}

MidiEvent::MidiEvent(llvm::Value *get, llvm::Type *type) : _get(get), _type(type) {
}

llvm::Value* MidiEvent::typePtr(Builder &builder) const {
    return builder.CreateStructGEP(_type, _get, 0, _get->getName() + ".type.ptr");
}

llvm::Value* MidiEvent::channelPtr(Builder &builder) const {
    return builder.CreateStructGEP(_type, _get, 1, _get->getName() + ".channel.ptr");
}

llvm::Value* MidiEvent::notePtr(Builder &builder) const {
    return builder.CreateStructGEP(_type, _get, 2, _get->getName() + ".note.ptr");
}

llvm::Value* MidiEvent::paramPtr(Builder &builder) const {
    return builder.CreateStructGEP(_type, _get, 3, _get->getName() + ".param.ptr");
}

llvm::Value* MidiEvent::type(Builder &builder) const {
    return builder.CreateLoad(typePtr(builder), _get->getName() + ".type");
}

llvm::Value* MidiEvent::channel(Builder &builder) const {
    return builder.CreateLoad(channelPtr(builder), _get->getName() + ".channel");
}

llvm::Value* MidiEvent::note(Builder &builder) const {
    return builder.CreateLoad(notePtr(builder), _get->getName() + ".note");
}

llvm::Value* MidiEvent::param(Builder &builder) const {
    return builder.CreateLoad(paramPtr(builder), _get->getName() + ".param");
}
