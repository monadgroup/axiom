#pragma once

#include <llvm/IR/IRBuilder.h>

namespace MaximCodegen {

    class Builder {
    public:
        explicit Builder(llvm::BasicBlock *block);
        explicit Builder(llvm::LLVMContext &context);

        void SetInsertPoint(llvm::BasicBlock *block);

        llvm::GetElementPtrInst *CreateGEP(llvm::Type *type, llvm::Value *ptr, llvm::ArrayRef<llvm::Value *> idxList,
                                           const llvm::Twine &name);

        llvm::ReturnInst *CreateRetVoid();

        llvm::ReturnInst *CreateRet(llvm::Value *v);

        llvm::BranchInst *CreateBr(llvm::BasicBlock *dest);

        llvm::BranchInst *CreateCondBr(llvm::Value *cond, llvm::BasicBlock *trueBr, llvm::BasicBlock *falseBr);

        llvm::Value *CreateAdd(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFAdd(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateSub(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFSub(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateMul(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFMul(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateUDiv(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateSDiv(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFDiv(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateURem(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateSRem(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFRem(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateAnd(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateOr(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateXor(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::AllocaInst *CreateAlloca(llvm::Type *ty, const llvm::Twine &name);

        llvm::AllocaInst *CreateAlloca(llvm::Type *ty, llvm::Value *arraySize, const llvm::Twine &name);

        llvm::LoadInst *CreateLoad(llvm::Value *ptr, const llvm::Twine &name);

        llvm::StoreInst *CreateStore(llvm::Value *val, llvm::Value *ptr);

        llvm::Value *CreateFPToUI(llvm::Value *v, llvm::Type *destTy, const llvm::Twine &name);

        llvm::Value *CreateFPToSI(llvm::Value *v, llvm::Type *destTy, const llvm::Twine &name);

        llvm::Value *CreateUIToFP(llvm::Value *v, llvm::Type *destTy, const llvm::Twine &name);

        llvm::Value *CreateSIToFP(llvm::Value *v, llvm::Type *destTy, const llvm::Twine &name);

        llvm::Value *CreateBitCast(llvm::Value *v, llvm::Type *destTy, const llvm::Twine &name);

        llvm::Value *CreateICmpUGE(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmpOEQ(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmpOGT(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmpOGE(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmpOLT(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmpOLE(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmpONE(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmpORD(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::CallInst *CreateCall(llvm::Function *callee, llvm::ArrayRef<llvm::Value*> args, const llvm::Twine &name);

        llvm::Value *CreateExtractElement(llvm::Value *vec, llvm::Value *idx, const llvm::Twine &name);

        llvm::Value *CreateExtractElement(llvm::Value *vec, uint64_t idx, const llvm::Twine &name);

        llvm::Value *CreateInsertElement(llvm::Value *vec, llvm::Value *newElt, llvm::Value *idx, const llvm::Twine &name);

        llvm::Value *CreateInsertElement(llvm::Value *vec, llvm::Value *newElt, uint64_t idx, const llvm::Twine &name);

        llvm::Value *CreateShuffleVector(llvm::Value *v1, llvm::Value *v2, llvm::ArrayRef<uint32_t> intMask, const llvm::Twine &name);

        llvm::Value *CreateExtractValue(llvm::Value *agg, llvm::ArrayRef<unsigned> idxs, const llvm::Twine &name);

        llvm::Value *CreateInsertValue(llvm::Value *agg, llvm::Value *val, llvm::ArrayRef<unsigned> idxs, const llvm::Twine &name);

        llvm::Value *CreateVectorSplat(unsigned numElts, llvm::Value *v, const llvm::Twine &name);

    private:
        llvm::IRBuilder<> _builder;
    };

}
