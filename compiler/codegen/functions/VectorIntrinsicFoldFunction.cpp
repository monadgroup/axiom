#include "VectorIntrinsicFoldFunction.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/InstrTypes.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

VectorIntrinsicFoldFunction::VectorIntrinsicFoldFunction(MaximContext *context, llvm::Intrinsic::ID id,
                                                         std::string name)
    : Function(context,
               std::move(name), context->numType(), {}, Parameter::create(context->numType(), false, false), nullptr),
      _id(id) {
}

std::unique_ptr<VectorIntrinsicFoldFunction> VectorIntrinsicFoldFunction::create(MaximContext *context,
                                                                             llvm::Intrinsic::ID id, std::string name) {
    return std::make_unique<VectorIntrinsicFoldFunction>(context, id, name);
}

std::unique_ptr<Value> VectorIntrinsicFoldFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                             std::unique_ptr<VarArg> vararg,
                                                             llvm::Value *funcContext, llvm::Function *func,
                                                             llvm::Module *module) {
    auto intrinsic = llvm::Intrinsic::getDeclaration(module, _id, {context()->numType()->vecType()});
    auto varCount = vararg->count(b);
    auto firstVal = vararg->atIndex((uint64_t) 0, b);
    auto firstNum = dynamic_cast<Num*>(firstVal.get());
    assert(firstNum);

    auto vecPtr = b.CreateAlloca(context()->numType()->vecType(), nullptr, "accum.vecptr");
    b.CreateStore(firstNum->vec(b), vecPtr);
    auto activePtr = b.CreateAlloca(context()->numType()->activeType(), nullptr, "accum.activeptr");
    b.CreateStore(firstNum->active(b), activePtr);

    auto indexType = llvm::Type::getInt8Ty(context()->llvm());
    auto indexPtr = b.CreateAlloca(indexType, nullptr, "accum.indexptr");
    b.CreateStore(llvm::ConstantInt::get(indexType, 0, false), indexPtr);

    auto loopCheckBlock = llvm::BasicBlock::Create(context()->llvm(), "loop.check", func);
    auto loopContinueBlock = llvm::BasicBlock::Create(context()->llvm(), "loop.continue", func);
    auto finishBlock = llvm::BasicBlock::Create(context()->llvm(), "finish", func);

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
    auto nextNum = dynamic_cast<Num*>(nextVal.get());
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
    auto undefPos = SourcePos(-1, -1);
    return firstNum->withVec(b, finalVec, undefPos, undefPos)->withActive(b, finalActive, undefPos, undefPos);
}

std::unique_ptr<Value> VectorIntrinsicFoldFunction::generateConst(Builder &b,
                                                                  std::vector<std::unique_ptr<Value>> params,
                                                                  std::unique_ptr<ConstVarArg> vararg,
                                                                  llvm::Value *funcContext, llvm::Function *func,
                                                                  llvm::Module *module) {
    auto intrinsic = llvm::Intrinsic::getDeclaration(module, _id, {context()->numType()->vecType()});
    auto firstVal = vararg->atIndex(0);
    auto firstNum = dynamic_cast<Num*>(firstVal.get());
    assert(firstNum);

    auto currentVec = firstNum->vec(b);
    auto currentActive = firstNum->active(b);

    for (size_t i = 1; i < vararg->count(); i++) {
        auto nextVal = vararg->atIndex(i);
        auto nextNum = dynamic_cast<Num*>(nextVal.get());
        assert(nextNum);
        auto nextVec = nextNum->vec(b);
        auto nextActive = nextNum->active(b);
        currentVec = CreateCall(b, intrinsic, {currentVec, nextVec}, "storevec");
        currentActive = b.CreateOr(currentActive, nextActive, "storeactive");
    }

    auto undefPos = SourcePos(-1, -1);
    return firstNum->withVec(b, currentVec, undefPos, undefPos)->withActive(b, currentActive, undefPos, undefPos);
}
