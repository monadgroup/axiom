#include "ChannelFunction.h"

#include "../MaximContext.h"
#include "../Midi.h"
#include "../Num.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

ChannelFunction::ChannelFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "channel", ctx->midiType(),
               {Parameter(ctx->midiType(), true, false),
                Parameter(ctx->numType(), false, false)},
               nullptr, true) {

}

std::unique_ptr<ChannelFunction> ChannelFunction::create(MaximContext *context, llvm::Module *module) {
    return std::make_unique<ChannelFunction>(context, module);
}

std::unique_ptr<Value> ChannelFunction::generate(ComposableModuleClassMethod *method,
                                                 const std::vector<std::unique_ptr<Value>> &params,
                                                 std::unique_ptr<VarArg> vararg) {
    auto minFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::minnum, {llvm::Type::getFloatTy(ctx()->llvm())});
    auto maxFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::maxnum, {llvm::Type::getFloatTy(ctx()->llvm())});

    auto inputMidi = dynamic_cast<Midi *>(params[0].get());
    auto channelNum = dynamic_cast<Num *>(params[1].get());

    auto maxChannels = powl(2, ctx()->midiType()->channelType()->getBitWidth());

    auto &b = method->builder();

    auto undefPos = SourcePos(-1, -1);
    auto result = Midi::create(ctx(), method->allocaBuilder(), undefPos, undefPos);
    result->setCount(b, (uint64_t) 0);
    result->setActive(b, inputMidi->active(b));

    auto channelFloat = b.CreateExtractElement(channelNum->vec(b), (uint64_t) 0, "channelfloat");
    channelFloat = CreateCall(b, maxFunc, {channelFloat, ctx()->constFloat(0)}, "channelfloat");
    channelFloat = CreateCall(b, minFunc, {channelFloat, ctx()->constFloat((float) maxChannels)}, "channelfloat");

    auto channelInt = b.CreateFPToUI(channelFloat, ctx()->midiType()->channelType(), "channelint");
    auto eventCount = inputMidi->count(b);
    auto currentIndexPtr = method->allocaBuilder().CreateAlloca(ctx()->midiType()->countType(), nullptr, "index.ptr");
    b.CreateStore(
        llvm::ConstantInt::get(ctx()->midiType()->countType(), 0, false),
        currentIndexPtr
    );

    auto func = method->get(method->moduleClass()->module());
    auto loopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopcheck", func);
    auto loopBodyBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopbody", func);
    auto loopMatchBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopmatch", func);
    auto loopEndBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopend", func);

    b.CreateBr(loopCheckBlock);
    b.SetInsertPoint(loopCheckBlock);

    auto currentIndex = b.CreateLoad(currentIndexPtr, "index");
    auto indexCond = b.CreateICmpULT(currentIndex, eventCount, "indexcond");
    b.CreateCondBr(indexCond, loopBodyBlock, loopEndBlock);

    b.SetInsertPoint(loopBodyBlock);
    auto nextIndex = b.CreateAdd(currentIndex, llvm::ConstantInt::get(ctx()->midiType()->countType(), 1, false), "nextindex");
    b.CreateStore(nextIndex, currentIndexPtr);

    auto currentEvent = inputMidi->eventAt(b, currentIndex);
    auto channelCond = b.CreateICmpEQ(currentEvent.channel(b), channelInt, "channelcond");
    b.CreateCondBr(channelCond, loopMatchBlock, loopCheckBlock);

    b.SetInsertPoint(loopMatchBlock);
    result->pushEvent(b, currentEvent, method->moduleClass()->module());
    b.CreateBr(loopCheckBlock);

    b.SetInsertPoint(loopEndBlock);
    return std::move(result);
}
