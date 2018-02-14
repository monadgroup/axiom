#pragma once

#include <llvm/IR/Intrinsics.h>

#include "../Function.h"

namespace MaximCodegen {

    class VectorIntrinsicFoldFunction : public Function {
    public:
        VectorIntrinsicFoldFunction(MaximContext *context, llvm::Intrinsic::ID id, std::string name);

        static std::unique_ptr<VectorIntrinsicFoldFunction> create(MaximContext *context, llvm::Intrinsic::ID id, std::string name);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;

        std::unique_ptr<Value> generateConst(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<ConstVarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;

    private:
        llvm::Intrinsic::ID _id;
    };

}
