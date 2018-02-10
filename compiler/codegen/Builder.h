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

        llvm::Value *CreateExtractValue(llvm::Value *agg, llvm::ArrayRef<unsigned> idxs, const llvm::Twine &name);

        llvm::Value *
        CreateInsertValue(llvm::Value *agg, llvm::Value *val, llvm::ArrayRef<unsigned> idxs, const llvm::Twine &name);

        llvm::Value *CreateGEP(llvm::Type *type, llvm::Value *val, llvm::ArrayRef<llvm::Value*> idxList, const llvm::Twine &name);

        llvm::Value *CreateLoad(llvm::Value *ptr, const llvm::Twine &name);

        llvm::Value *CreateStore(llvm::Value *val, llvm::Value *ptr);

        llvm::Value *CreateAlloca(llvm::Type *type, const llvm::Twine &name);

        llvm::Value *CreateAlloca(llvm::Type *type, llvm::Value *arraySize, const llvm::Twine &name);

        llvm::Value *CreateCall(llvm::Function *func, llvm::ArrayRef<llvm::Value*> args, const Twine &name);

        void CreateRet(llvm::Value *v);
    };

}
