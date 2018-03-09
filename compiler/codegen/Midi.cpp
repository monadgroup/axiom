#include "Midi.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Midi::Midi(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context ){

}

std::unique_ptr<Midi> Midi::create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Midi>(context, get, startPos, endPos);
}

llvm::Value* Midi::count(Builder &builder) const {
    return builder.CreateExtractValue(_get, {0}, _get->getName() + ".count");
}

llvm::Value* Midi::events(Builder &builder) const {
    return builder.CreateExtractValue(_get, {1}, _get->getName() + ".array");
}

MidiEvent Midi::eventAt(Builder &builder, unsigned index) const {
    return MidiEvent(
        builder.CreateExtractValue(_get, {1, index}, _get->getName() + ".event" + std::to_string(index))
    );
}

MidiEvent Midi::eventAt(Builder &builder, Builder &allocaBuilder, llvm::Value *index) const {
    auto arr = events(builder);
    auto arrPtr = allocaBuilder.CreateAlloca(arr->getType(), nullptr, _get->getName() + ".events");
    builder.CreateStore(arr, arrPtr);
    auto eventPtr = builder.CreateGEP(arrPtr, {
        _context->constInt(64, 0, false),
        index
    }, _get->getName() + ".event.ptr");

    return MidiEvent(
        builder.CreateLoad(eventPtr, _get->getName() + ".event")
    );
}

std::unique_ptr<Midi> Midi::withCount(Builder &builder, llvm::Value *count, SourcePos startPos, SourcePos endPos) const {
    return create(
        _context,
        builder.CreateInsertValue(_get, count, {0}, _get->getName() + ".new_count"),
        startPos, endPos
    );
}

std::unique_ptr<Midi> Midi::withEvents(Builder &builder, llvm::Value *events, SourcePos startPos,
                                       SourcePos endPos) const {
    return create(
        _context,
        builder.CreateInsertValue(_get, events, {1}, _get->getName() + ".new_events"),
        startPos, endPos
    );
}

std::unique_ptr<Midi> Midi::withEvent(Builder &builder, unsigned index, const MidiEvent &event, SourcePos startPos,
                                      SourcePos endPos) const {
    return create(
        _context,
        builder.CreateInsertValue(_get, event.get(), {1, index}, _get->getName() + ".new_event" + std::to_string(index)),
        startPos, endPos
    );
}

std::unique_ptr<Midi> Midi::withEvent(Builder &builder, Builder &allocaBuilder, llvm::Value *index, const MidiEvent &event, SourcePos startPos,
                                      SourcePos endPos) const {
    auto arr = events(builder);
    auto arrPtr = allocaBuilder.CreateAlloca(arr->getType(), nullptr, _get->getName() + ".events");
    builder.CreateStore(arr, arrPtr);
    auto eventPtr = builder.CreateGEP(arrPtr, {
        _context->constInt(64, 0, false),
        index
    }, _get->getName() + ".event.ptr");
    builder.CreateStore(event.get(), eventPtr);

    return withEvents(builder, builder.CreateLoad(eventPtr, _get->getName() + ".events.new"), startPos, endPos);
}

std::unique_ptr<Value> Midi::withSource(SourcePos startPos, SourcePos endPos) const {
    return create(_context, _get, startPos, endPos);
}

MidiType* Midi::type() const {
    return _context->midiType();
}

MidiEvent::MidiEvent(llvm::Value *get) : _get(get) {
}

llvm::Value* MidiEvent::type(Builder &builder) const {
    return builder.CreateExtractValue(_get, {0}, _get->getName() + ".type");
}

llvm::Value* MidiEvent::channel(Builder &builder) const {
    return builder.CreateExtractValue(_get, {1}, _get->getName() + ".channel");
}

llvm::Value* MidiEvent::note(Builder &builder) const {
    return builder.CreateExtractValue(_get, {2}, _get->getName() + ".note");
}

llvm::Value* MidiEvent::param(Builder &builder) const {
    return builder.CreateExtractValue(_get, {3}, _get->getName() + ".param");
}

MidiEvent MidiEvent::withType(Builder &builder, llvm::Value *type) const {
    return MidiEvent(builder.CreateInsertValue(_get, type, {0}, _get->getName() + ".new_type"));
}

MidiEvent MidiEvent::withChannel(Builder &builder, llvm::Value *channel) const {
    return MidiEvent(builder.CreateInsertValue(_get, channel, {1}, _get->getName() + ".new_channel"));
}

MidiEvent MidiEvent::withNote(Builder &builder, llvm::Value *note) const {
    return MidiEvent(builder.CreateInsertValue(_get, note, {2}, _get->getName() + ".new_note"));
}

MidiEvent MidiEvent::withParam(Builder &builder, llvm::Value *param) const {
    return MidiEvent(builder.CreateInsertValue(_get, param, {3}, _get->getName() + ".new_param"));
}
