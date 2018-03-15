#include "VoicesFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Midi.h"
#include "../Num.h"
#include "../MidiType.h"
#include "../Array.h"
#include "../../util.h"

using namespace MaximCodegen;

VoicesFunction::VoicesFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "voices", ctx->getArrayType(ctx->midiType()),
               {Parameter(ctx->midiType(), false),
                Parameter(ctx->getArrayType(ctx->numType()), false)},
               nullptr) {
}

std::unique_ptr<VoicesFunction> VoicesFunction::create(MaximContext *context, llvm::Module *module) {
    return std::make_unique<VoicesFunction>(context, module);
}

std::unique_ptr<Value> VoicesFunction::generate(ComposableModuleClassMethod *method,
                                                const std::vector<std::unique_ptr<Value>> &params,
                                                std::unique_ptr<VarArg> vararg) {
    auto inputVal = dynamic_cast<Midi *>(params[0].get());
    auto lastActiveVal = dynamic_cast<Array *>(params[1].get());
    assert(inputVal && lastActiveVal);

    auto &b = method->builder();
    auto currentNotesPtr = method->getEntryPointer(addEntry(llvm::ArrayType::get(
        ctx()->midiType()->noteType(),
        ArrayType::arraySize
    )), "ctx");

    auto undefPos = SourcePos(-1, -1);
    auto result = Array::create(ctx(), ctx()->getArrayType(ctx()->midiType()), method->allocaBuilder(), undefPos, undefPos);
    // todo: we might not need this, probly shouldn't have it cause performance
    ctx()->clearPtr(b, result->get());

    auto eventIndexPtr = method->allocaBuilder().CreateAlloca(llvm::Type::getInt8Ty(ctx()->llvm()), nullptr, "eventindex.ptr");
    auto innerIndexPtr = method->allocaBuilder().CreateAlloca(llvm::Type::getInt8Ty(ctx()->llvm()), nullptr, "innerindex.ptr");

    b.CreateStore(ctx()->constInt(8, 0, false), eventIndexPtr);

    auto eventCount = inputVal->count(b);

    auto func = method->get(method->moduleClass()->module());
    auto eventLoopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopcheck", func);
    auto eventLoopRunBlock = llvm::BasicBlock::Create(ctx()->llvm(), "looprun", func);
    auto eventLoopFinishBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopfinish", func);

    auto noteOnLoopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteonloopcheck", func);
    auto noteOnLoopRunBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteonlooprun", func);
    auto noteOnAssignBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteonassign", func);

    auto noteElseLoopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteelseloopcheck", func);
    auto noteElseLoopRunBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteelselooprun", func);
    auto noteElseAssignBlock = llvm::BasicBlock::Create(ctx()->llvm(), "noteelseassignblock", func);

    b.CreateBr(eventLoopCheckBlock);
    b.SetInsertPoint(eventLoopCheckBlock);

    auto currentEventIndex = b.CreateLoad(eventIndexPtr);
    // todo: shouldn't compare against maxEvents, but instead against current number of events!
    auto eventCond = b.CreateICmpULT(currentEventIndex, eventCount, "eventcond");
    b.CreateCondBr(eventCond, eventLoopRunBlock, eventLoopFinishBlock);
    b.SetInsertPoint(eventLoopRunBlock);

    auto incrementedEventIndex = b.CreateAdd(currentEventIndex, ctx()->constInt(8, 1, false), "nexteventindex");
    b.CreateStore(incrementedEventIndex, eventIndexPtr);
    auto currentEvent = inputVal->eventAt(b, currentEventIndex);
    auto isOnCond = b.CreateICmpEQ(
        currentEvent.type(b),
        llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::NOTE_ON),
        "oncond"
    );
    b.CreateStore(ctx()->constInt(8, 0, false), innerIndexPtr);

    b.CreateCondBr(isOnCond, noteOnLoopCheckBlock, noteElseLoopCheckBlock);

    {
        b.SetInsertPoint(noteOnLoopCheckBlock);
        auto currentActiveIndex = b.CreateLoad(innerIndexPtr);
        auto activeCond = b.CreateICmpULT(currentActiveIndex, ctx()->constInt(8, ArrayType::arraySize, false),
                                          "activecond");
        b.CreateCondBr(activeCond, noteOnLoopRunBlock, eventLoopCheckBlock);
        b.SetInsertPoint(noteOnLoopRunBlock);

        auto incrementedActiveIndex = b.CreateAdd(currentActiveIndex, ctx()->constInt(8, 1, false), "nextactiveindex");
        b.CreateStore(incrementedActiveIndex, innerIndexPtr);

        auto activeNum = AxiomUtil::strict_unique_cast<Num>(
            lastActiveVal->atIndex(currentActiveIndex, b).value(b, undefPos, undefPos));
        auto activeVal = b.CreateExtractElement(activeNum->vec(b), (uint64_t) 0, "activeval");
        auto notActiveCond = b.CreateFCmpOEQ(activeVal, ctx()->constFloat(0), "notactivecond");
        b.CreateCondBr(notActiveCond, noteOnAssignBlock, noteOnLoopCheckBlock);
        b.SetInsertPoint(noteOnAssignBlock);

        auto assignNotePtr = b.CreateGEP(currentNotesPtr, {
            ctx()->constInt(64, 0, false),
            currentActiveIndex
        }, "noteassign.ptr");
        b.CreateStore(currentEvent.note(b), assignNotePtr);
        auto targetMidi = AxiomUtil::strict_unique_cast<Midi>(result->atIndex(currentActiveIndex, b).value(b, undefPos, undefPos));
        targetMidi->pushEvent(b, currentEvent, method->moduleClass()->module());
        b.CreateBr(eventLoopCheckBlock); // break back to event loop
    }

    {
        b.SetInsertPoint(noteElseLoopCheckBlock);

        auto currentNoteIndex = b.CreateLoad(innerIndexPtr);
        auto noteCond = b.CreateICmpULT(currentNoteIndex, ctx()->constInt(8, ArrayType::arraySize, false), "notecond");
        b.CreateCondBr(noteCond, noteElseLoopRunBlock, eventLoopCheckBlock);
        b.SetInsertPoint(noteElseLoopRunBlock);

        auto incrementedNoteIndex = b.CreateAdd(currentNoteIndex, ctx()->constInt(8, 1, false), "noteindex");
        b.CreateStore(incrementedNoteIndex, innerIndexPtr);

        auto activeNum = AxiomUtil::strict_unique_cast<Num>(
            lastActiveVal->atIndex(currentNoteIndex, b).value(b, undefPos, undefPos));
        auto activeVal = b.CreateExtractElement(activeNum->vec(b), (uint64_t) 0, "activeval");
        auto activeCond = b.CreateFCmpONE(activeVal, ctx()->constFloat(0), "activecond");

        auto currentNotePtr = b.CreateGEP(currentNotesPtr, {
            ctx()->constInt(64, 0, false),
            currentNoteIndex
        }, "noteindex.ptr");
        auto currentNote = b.CreateLoad(currentNotePtr, "noteindex");
        auto notesEqualCond = b.CreateICmpEQ(currentNote, currentEvent.note(b), "notecond");

        auto currentEventType = currentEvent.type(b);
        auto channelAftertouchCond = b.CreateICmpEQ(
            currentEventType,
            llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::CHANNEL_AFTERTOUCH, false),
            "channelaftertouchcond"
        );
        auto channelPitchwheelCond = b.CreateICmpEQ(
            currentEventType,
            llvm::ConstantInt::get(ctx()->midiType()->typeType(), (uint64_t) MaximCommon::MidiEventType::PITCH_WHEEL, false),
            "channelpitchweelcond"
        );
        auto isChannelEventCond = b.CreateOr(channelAftertouchCond, channelPitchwheelCond, "channeleventcond");

        auto branchCond = b.CreateAnd(activeCond, b.CreateOr(notesEqualCond, isChannelEventCond, "matchescond"), "queuecond");

        b.CreateCondBr(branchCond, noteElseAssignBlock, noteElseLoopCheckBlock);
        b.SetInsertPoint(noteElseAssignBlock);
        auto targetMidi = AxiomUtil::strict_unique_cast<Midi>(result->atIndex(currentNoteIndex, b).value(b, undefPos, undefPos));
        targetMidi->pushEvent(b, currentEvent, method->moduleClass()->module());
        b.CreateBr(noteElseLoopCheckBlock);
    }

    b.SetInsertPoint(eventLoopFinishBlock);
    return std::move(result);
}
