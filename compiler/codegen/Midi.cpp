#include "Midi.h"

#include "MaximContext.h"

using namespace MaximCodegen;

Midi::Midi(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _get(get), _context(context ){

}

Midi::Midi(MaximContext *context, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos)
    : Value(startPos, endPos), _context(context) {
    _get = allocaBuilder.CreateAlloca(type()->get(), nullptr, "midi");
}

std::unique_ptr<Midi> Midi::create(MaximContext *context, llvm::Value *get, SourcePos startPos, SourcePos endPos) {
    return std::make_unique<Midi>(context, get, startPos, endPos);
}

std::unique_ptr<Midi> Midi::create(MaximContext *context, Builder &allocaBuilder, SourcePos startPos,
                                   SourcePos endPos) {
    return std::make_unique<Midi>(context, allocaBuilder, startPos, endPos);
}

void Midi::initialize(llvm::Module *module, MaximContext *ctx) {
    auto func = pushEventFunc(module, ctx);
    auto entryBlock = llvm::BasicBlock::Create(ctx->llvm(), "entry", func);
    auto canPushBlock = llvm::BasicBlock::Create(ctx->llvm(), "canpush", func);
    auto endBlock = llvm::BasicBlock::Create(ctx->llvm(), "end", func);
    Builder b(entryBlock);

    auto undefPos = SourcePos(-1, -1);
    Midi currentMidi(ctx, func->arg_begin(), undefPos, undefPos);
    auto currentCount = currentMidi.count(b);
    auto canPushCond = b.CreateICmpULT(
        currentCount,
        llvm::ConstantInt::get(ctx->midiType()->countType(), MidiType::maxEvents, false),
        "canpushcond"
    );
    b.CreateCondBr(canPushCond, canPushBlock, endBlock);
    b.SetInsertPoint(canPushBlock);

    ctx->copyPtr(b, func->arg_begin() + 1, currentMidi.eventPtr(b, currentCount));
    auto newCount = b.CreateAdd(
        currentCount,
        llvm::ConstantInt::get(ctx->midiType()->countType(), 1, false),
        "newcount"
    );
    currentMidi.setCount(b, newCount);
    b.CreateBr(endBlock);

    b.SetInsertPoint(endBlock);
    b.CreateRetVoid();
}

llvm::Value* Midi::activePtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 0, _get->getName() + ".active.ptr");
}

llvm::Value* Midi::countPtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 1, _get->getName() + ".count.ptr");
}

llvm::Value* Midi::eventsPtr(Builder &builder) const {
    return builder.CreateStructGEP(type()->get(), _get, 2, _get->getName() + ".events.ptr");
}

llvm::Value* Midi::eventPtr(Builder &builder, size_t index) const {
    return eventPtr(builder, _context->constInt(32, index, false));
}

llvm::Value* Midi::eventPtr(Builder &builder, llvm::Value *index) const {
    return builder.CreateGEP(
        _get,
        {
            _context->constInt(64, 0, false),
            _context->constInt(32, 2, false),
            index
        },
        _get->getName() + ".event.ptr"
    );
}

llvm::Value* Midi::active(Builder &builder) {
    return builder.CreateLoad(activePtr(builder), _get->getName() + ".active");
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

void Midi::setActive(Builder &builder, bool active) const {
    setActive(builder, llvm::ConstantInt::get(type()->activeType(), (uint64_t) active, false));
}

void Midi::setActive(Builder &builder, llvm::Value *active) const {
    builder.CreateStore(active, activePtr(builder));
}

void Midi::setCount(Builder &builder, uint64_t count) const {
    setCount(builder, llvm::ConstantInt::get(type()->countType(), count, false));
}

void Midi::setCount(Builder &builder, llvm::Value *count) const {
    builder.CreateStore(count, countPtr(builder));
}

void Midi::pushEvent(Builder &builder, const MidiEvent &event, llvm::Module *module) const {
    CreateCall(builder, pushEventFunc(module, _context), {_get, event.get()}, "");
}

std::unique_ptr<Value> Midi::withSource(SourcePos startPos, SourcePos endPos) const {
    return create(_context, _get, startPos, endPos);
}

MidiType* Midi::type() const {
    return _context->midiType();
}

llvm::Function* Midi::pushEventFunc(llvm::Module *module, MaximContext *ctx) {
    auto funcName = "maxim.midi.pushEvent";
    if (auto func = module->getFunction(funcName)) {
        return func;
    }

    return llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {
            llvm::PointerType::get(ctx->midiType()->get(), 0),
            llvm::PointerType::get(ctx->midiType()->eventType(), 0)
        }, false),
        llvm::Function::LinkageTypes::InternalLinkage,
        funcName, module
    );
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
