#pragma once

#include <llvm/ADT/Twine.h>

namespace llvm {
    class Value;

    class Function;

    class BasicBlock;
}

namespace MaximCodegen {

    class Builder {
    public:
        Builder(llvm::BasicBlock *block);

        void SetInsertPoint(llvm::BasicBlock *block);

        llvm::Value *CreateExtractValue(llvm::Value *agg, llvm::ArrayRef<unsigned> idxs, const llvm::Twine &name);

        llvm::Value *
        CreateInsertValue(llvm::Value *agg, llvm::Value *val, llvm::ArrayRef<unsigned> idxs, const llvm::Twine &name);

        llvm::Value *CreateExtractElement(llvm::Value *vec, llvm::Value *idx, const llvm::Twine &name);

        llvm::Value *CreateExtractElement(llvm::Value *vec, uint64_t idx, const llvm::Twine &name);

        llvm::Value *CreateInsertElement(llvm::Value *vec, llvm::Value *newElt, llvm::Value *idx, const llvm::Twine &name);

        llvm::Value *CreateInsertElement(llvm::Value *vec, llvm::Value *newElt, uint64_t idx, const llvm::Twine &name);

        llvm::Value *
        CreateGEP(llvm::Type *type, llvm::Value *val, llvm::ArrayRef<llvm::Value *> idxList, const llvm::Twine &name);

        llvm::Value *CreateLoad(llvm::Value *ptr, const llvm::Twine &name);

        llvm::Value *CreateStore(llvm::Value *val, llvm::Value *ptr);

        llvm::Value *CreateAlloca(llvm::Type *type, const llvm::Twine &name);

        llvm::Value *CreateAlloca(llvm::Type *type, llvm::Value *arraySize, const llvm::Twine &name);

        llvm::Value *CreateCall(llvm::Function *func, llvm::ArrayRef<llvm::Value *> args, const llvm::Twine &name);

        llvm::Value *CreateAnd(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateOr(llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFCmp(llvm::CmpInst::Predicate p, llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateICmp(llvm::CmpInst::Predicate p, llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateBinOp(llvm::Instruction::BinaryOps opc, llvm::Value *lhs, llvm::Value *rhs, const llvm::Twine &name);

        llvm::Value *CreateFPToSI(llvm::Value *val, llvm::Type *type, const llvm::Twine &name);

        llvm::Value *CreateFPToUI(llvm::Value *val, llvm::Type *type, const llvm::Twine &name);

        llvm::Value *CreateSIToFP(llvm::Value *val, llvm::Type *type, const llvm::Twine &name);

        llvm::Value *CreateUIToFP(llvm::Value *val, llvm::Type *type, const llvm::Twine &name);

        void CreateBr(llvm::BasicBlock *dest);

        void CreateCondBr(llvm::Value *cond, llvm::BasicBlock *t, llvm::BasicBlock *f);

        void CreateRet(llvm::Value *v);
    };

}
