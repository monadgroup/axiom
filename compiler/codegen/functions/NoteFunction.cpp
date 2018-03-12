#include "NoteFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Midi.h"
#include "../Num.h"
#include "../Tuple.h"

using namespace MaximCodegen;

NoteFunction::NoteFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "note", ctx->getTupleType({ctx->numType(), ctx->numType(), ctx->numType()}), {Parameter(ctx->midiType(), false)},
               nullptr) {

}

std::unique_ptr<NoteFunction> NoteFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<NoteFunction>(ctx, module);
}

std::unique_ptr<Value> NoteFunction::generate(ComposableModuleClassMethod *method,
                                              const std::vector<std::unique_ptr<Value>> &params,
                                              std::unique_ptr<VarArg> vararg) {
    auto paramVal = dynamic_cast<Midi *>(params[0].get());
    assert(paramVal);

    auto lastNotePtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->constFloat(0)), "lastnote.ptr");
    auto lastPitchPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->constFloat(0)), "lastpitch.ptr");
    auto lastVelocityPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->constFloat(0)), "lastvelocity.ptr");
    auto lastAftertouchPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->constFloat(0)), "lastaftertouch.ptr");
    auto activeCountPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->constInt(8, 0, false)), "activecount.ptr");

    auto &b = method->builder();

    auto iterCount = paramVal->count(b);
    auto indexPtr = b.CreateAlloca(iterCount->getType(), nullptr, "index.ptr");
    b.CreateStore(llvm::ConstantInt::get(iterCount->getType(), 0, false), indexPtr);

    auto func = method->get(method->moduleClass()->module());
    auto loopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loop.check", func);
    auto loopContinueBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loop.continue", func);
    auto loopEndBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loop.end", func);
    auto finishBlock = llvm::BasicBlock::Create(ctx()->llvm(), "finish", func);

    b.CreateBr(loopCheckBlock);
    b.SetInsertPoint(loopCheckBlock);

    auto currentIndex = b.CreateLoad(indexPtr, "currentindex");
    auto branchCond = b.CreateICmp(
        llvm::CmpInst::Predicate::ICMP_ULT,
        currentIndex,
        iterCount,
        "branchcond"
    );

    b.CreateCondBr(branchCond, loopContinueBlock, finishBlock);
    b.SetInsertPoint(loopContinueBlock);

    auto event = paramVal->eventAt(b, currentIndex);
    auto eventType = event.type(b);
    auto eventSwitch = b.CreateSwitch(eventType, loopEndBlock, 3);

    // NOTE ON EVENT:
    //  - set lastNote to event note
    //  - set lastVelocity to normalized event velocity
    //  - increment activeCount
    auto noteOnBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteon", func);
    eventSwitch->addCase(
        llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::NOTE_ON, false),
        noteOnBlock
    );
    b.SetInsertPoint(noteOnBlock);

    auto floatNote = b.CreateUIToFP(event.note(b), llvm::Type::getFloatTy(ctx()->llvm()), "note.float");
    b.CreateStore(floatNote, lastNotePtr);
    auto floatVelocity = b.CreateUIToFP(event.param(b), llvm::Type::getFloatTy(ctx()->llvm()), "velocity.float");
    auto normalizedVelocity = b.CreateFDiv(floatVelocity, ctx()->constFloat(127), "velocity.normalized");
    b.CreateStore(normalizedVelocity, lastVelocityPtr);
    auto incrementedActive = b.CreateAdd(
        b.CreateLoad(activeCountPtr, "activecount"),
        ctx()->constInt(8, 1, false),
        "active.incremented"
    );
    b.CreateStore(incrementedActive, activeCountPtr);
    b.CreateBr(loopEndBlock);

    // NOTE OFF EVENT:
    //  - decrement activeCount
    auto noteOffBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteoff", func);
    eventSwitch->addCase(
        llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::NOTE_OFF, false),
        noteOffBlock
    );
    b.SetInsertPoint(noteOffBlock);

    auto decrementedActive = b.CreateSub(
        b.CreateLoad(activeCountPtr, "activecount"),
        ctx()->constInt(8, 1, false),
        "active.decremented"
    );
    b.CreateStore(decrementedActive, activeCountPtr);
    b.CreateBr(loopEndBlock);

    // PITCH WHEEL EVENT:
    //  - set lastPitchwheel to normalized pitch wheel
    //    note: pitch wheel LSB stored in 4 bits of note, MSB stored in 4 bits of param
    auto pitchWheelBlock = llvm::BasicBlock::Create(ctx()->llvm(), "pitchwheel", func);
    eventSwitch->addCase(
        llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::PITCH_WHEEL, false),
        pitchWheelBlock
    );
    b.SetInsertPoint(pitchWheelBlock);

    auto msbInt = b.CreateZExt(event.param(b), llvm::Type::getIntNTy(ctx()->llvm(), 14), "msbint");
    msbInt = b.CreateShl(msbInt, 7, "msbint");
    auto lsbInt = b.CreateZExt(event.note(b), llvm::Type::getIntNTy(ctx()->llvm(), 14), "lsbint");
    auto fullPitch = b.CreateAdd(msbInt, lsbInt, "pitch");
    auto pitchFloat = b.CreateUIToFP(fullPitch, llvm::Type::getFloatTy(ctx()->llvm()), "pitch.float");
    auto normalizedPitch = b.CreateFDiv(
        pitchFloat,
        ctx()->constFloat(8192),
        "pitch.normalized"
    );
    normalizedPitch = b.CreateFSub(
        normalizedPitch,
        ctx()->constFloat(1),
        "pitch.normalized"
    );
    b.CreateStore(normalizedPitch, lastPitchPtr);
    b.CreateBr(loopEndBlock);

    // AFTERTOUCH EVENT:
    //  - set lastAftertouch to normalized aftertouch
    auto aftertouchBlock = llvm::BasicBlock::Create(ctx()->llvm(), "aftertouch", func);
    eventSwitch->addCase(
        llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::POLYPHONIC_AFTERTOUCH, false),
        aftertouchBlock
    );
    eventSwitch->addCase(
        llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::CHANNEL_AFTERTOUCH, false),
        aftertouchBlock
    );
    b.SetInsertPoint(aftertouchBlock);

    auto aftertouchFloat = b.CreateUIToFP(event.param(b), llvm::Type::getFloatTy(ctx()->llvm()), "aftertouch.float");
    auto normalizedAftertouch = b.CreateFDiv(
        aftertouchFloat,
        ctx()->constFloat(127),
        "aftertouch.normalized"
    );
    b.CreateStore(normalizedAftertouch, lastAftertouchPtr);
    b.CreateBr(loopEndBlock);

    b.SetInsertPoint(loopEndBlock);
    auto incrIndex = b.CreateAdd(
        currentIndex,
        llvm::ConstantInt::get(iterCount->getType(), 1, false),
        "index.incremented"
    );
    b.CreateStore(incrIndex, indexPtr);
    b.CreateBr(loopCheckBlock);

    b.SetInsertPoint(finishBlock);

    auto isActive = b.CreateICmpUGT(
        b.CreateLoad(activeCountPtr, "activecount"),
        ctx()->constInt(8, 0, false),
        "active"
    );

    auto noteNum = b.CreateFAdd(
        b.CreateLoad(lastNotePtr, "lastnote"),
        b.CreateLoad(lastPitchPtr, "lastpitch"),
        "notenum"
    );
    auto velocityNum = b.CreateLoad(lastVelocityPtr, "velocitynum");
    auto aftertouchNum = b.CreateLoad(lastAftertouchPtr, "aftertoucnum");

    auto undefPos = SourcePos(-1, -1);

    Tuple::Storage tupleVecs;
    auto resultNote = Num::create(ctx(), method->allocaBuilder(), undefPos, undefPos);
    resultNote->setVec(b, b.CreateVectorSplat(2, noteNum, "notevec"));
    resultNote->setForm(b, MaximCommon::FormType::NOTE);
    resultNote->setActive(b, isActive);
    tupleVecs.push_back(std::move(resultNote));

    auto resultVelocity = Num::create(ctx(), method->allocaBuilder(), undefPos, undefPos);
    resultVelocity->setVec(b, b.CreateVectorSplat(2, velocityNum, "velocityvec"));
    resultVelocity->setForm(b, MaximCommon::FormType::LINEAR);
    resultVelocity->setActive(b, isActive);
    tupleVecs.push_back(std::move(resultVelocity));

    auto resultAftertouch = Num::create(ctx(), method->allocaBuilder(), undefPos, undefPos);
    resultAftertouch->setVec(b, b.CreateVectorSplat(2, aftertouchNum, "aftertouchvec"));
    resultAftertouch->setForm(b, MaximCommon::FormType::LINEAR);
    resultAftertouch->setActive(b, isActive);
    tupleVecs.push_back(std::move(resultAftertouch));

    return Tuple::create(
        ctx(),
        std::move(tupleVecs),
        b, method->allocaBuilder(), undefPos, undefPos
    );
}
