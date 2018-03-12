#include "VectorIntrinsicFoldFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

VectorIntrinsicFoldFunction::VectorIntrinsicFoldFunction(MaximContext *ctx, llvm::Module *module,
                                                         llvm::Intrinsic::ID id, std::string name)
    : Function(ctx, module, std::move(name), ctx->numType(), {}, Parameter::create(ctx->numType(), false, false)),
      _id(id) {

}

std::unique_ptr<VectorIntrinsicFoldFunction> VectorIntrinsicFoldFunction::create(MaximContext *ctx,
                                                                                 llvm::Module *module,
                                                                                 llvm::Intrinsic::ID id,
                                                                                 std::string name) {
    return std::make_unique<VectorIntrinsicFoldFunction>(ctx, module, id, name);
}

std::unique_ptr<Value> VectorIntrinsicFoldFunction::generate(ComposableModuleClassMethod *method,
                                                             const std::vector<std::unique_ptr<Value>> &params,
                                                             std::unique_ptr<VarArg> vararg) {
    auto intrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), _id,
                                                     {ctx()->numType()->vecType()});
    auto &b = method->builder();

    auto varCount = vararg->count(b);
    auto firstVal = vararg->atIndex((uint64_t) 0, b);
    auto firstNum = dynamic_cast<Num *>(firstVal.get());
    assert(firstNum);

    auto vecPtr = b.CreateAlloca(ctx()->numType()->vecType(), nullptr, "accum.vecptr");
    b.CreateStore(firstNum->vec(b), vecPtr);
    auto activePtr = b.CreateAlloca(ctx()->numType()->activeType(), nullptr, "accum.activeptr");
    b.CreateStore(firstNum->active(b), activePtr);

    auto indexType = llvm::Type::getInt8Ty(ctx()->llvm());
    auto indexPtr = b.CreateAlloca(indexType, nullptr, "accum.indexptr");
    b.CreateStore(llvm::ConstantInt::get(indexType, 0, false), indexPtr);

    auto func = method->get(method->moduleClass()->module());
    auto loopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loop.check", func);
    auto loopContinueBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loop.continue", func);
    auto finishBlock = llvm::BasicBlock::Create(ctx()->llvm(), "finish", func);

    b.CreateBr(loopCheckBlock);
    b.SetInsertPoint(loopCheckBlock);

    auto incrementedIndex = b.CreateBinOp(
        llvm::Instruction::BinaryOps::Add,
        b.CreateLoad(indexPtr, "lastindex"),
        llvm::ConstantInt::get(indexType, 1, false),
        "nextindex"
    );
    auto branchCond = b.CreateICmp(
        llvm::CmpInst::Predicate::ICMP_ULT,
        incrementedIndex,
        varCount,
        "branchcond"
    );

    b.CreateCondBr(branchCond, loopContinueBlock, finishBlock);
    b.SetInsertPoint(loopContinueBlock);

    b.CreateStore(incrementedIndex, indexPtr);
    auto lastVec = b.CreateLoad(vecPtr, "lastvec");
    auto nextVal = vararg->atIndex(incrementedIndex, b);
    auto nextNum = dynamic_cast<Num *>(nextVal.get());
    assert(nextNum);
    auto nextVec = nextNum->vec(b);

    auto storeVec = CreateCall(b, intrinsic, {lastVec, nextVec}, "storevec");
    b.CreateStore(storeVec, vecPtr);

    b.CreateStore(b.CreateOr(
        b.CreateLoad(activePtr, "lastactive"),
        nextNum->active(b),
        "storeactive"
    ), activePtr);

    b.CreateBr(loopCheckBlock);
    b.SetInsertPoint(finishBlock);

    auto finalVec = b.CreateLoad(vecPtr, "finalvec");
    auto finalActive = b.CreateLoad(activePtr, "finalactive");
    firstNum->setVec(b, finalVec);
    firstNum->setActive(b, finalActive);
    return firstNum->clone();
}
