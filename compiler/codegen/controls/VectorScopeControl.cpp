#include "VectorScopeControl.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

VectorScopeControl::VectorScopeControl(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Control(ctx, module, MaximCommon::ControlType::SCOPE, ctx->vecScopeStorage(), ctx->numType()->get(), ctx->numType()->get(), "vectorcontrol") {

    auto valueField = addField("value", ctx->numType());

    auto getMethod = valueField->getValue();
    getMethod->builder().CreateRet(getMethod->groupPtr());

    auto setMethod = valueField->setValue();
    ctx->copyPtr(setMethod->builder(), setMethod->arg(0), setMethod->groupPtr());

    auto &b = update()->builder();
    auto posPtr = b.CreateStructGEP(dataType(), update()->storagePtr(), 0, "bufferposptr");
    auto capacityVal = b.CreateLoad(b.CreateStructGEP(dataType(), update()->storagePtr(), 1, "buffercapptr"), "buffercap");
    auto bufferPtr = b.CreateStructGEP(dataType(), update()->storagePtr(), 2, "bufferptr");

    auto currentPos = b.CreateLoad(posPtr, "bufferpos");
    currentPos->setAtomic(llvm::AtomicOrdering::SequentiallyConsistent);
    currentPos->setAlignment(16);
    auto canWrite = b.CreateICmpULT(currentPos, capacityVal, "canwrite");

    auto canWriteBlock = llvm::BasicBlock::Create(ctx->llvm(), "canwrite", update()->get(module));
    auto endBlock = llvm::BasicBlock::Create(ctx->llvm(), "end", update()->get(module));
    b.CreateCondBr(canWrite, canWriteBlock, endBlock);
    b.SetInsertPoint(canWriteBlock);

    auto writePosPtr = b.CreateInBoundsGEP(bufferPtr->getType()->getPointerElementType(), bufferPtr, {
        ctx->constInt(64, 0, false),
        currentPos
    }, "writeptr");
    auto writeNum = Num::create(ctx, update()->groupPtr(), SourcePos(-1, -1), SourcePos(-1, -1));
    b.CreateStore(writeNum->vec(b), writePosPtr);

    auto newPos = b.CreateAdd(currentPos, ctx->constInt(16, 1, false), "newpos");
    b.CreateAtomicCmpXchg(posPtr, currentPos, newPos, llvm::AtomicOrdering::SequentiallyConsistent, llvm::AtomicOrdering::SequentiallyConsistent);
    b.CreateBr(endBlock);
    b.SetInsertPoint(endBlock);

    complete();
}

std::unique_ptr<VectorScopeControl> VectorScopeControl::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<VectorScopeControl>(ctx, module);
}
