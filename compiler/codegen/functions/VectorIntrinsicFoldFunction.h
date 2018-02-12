#pragma once

#include <llvm/IR/Intrinsics.h>

#include "../Function.h"

namespace MaximCodegen {

    class VectorIntrinsicFoldFunction : public Function {
    public:
        VectorIntrinsicFoldFunction(MaximContext *context, llvm::Intrinsic::ID id, std::string name, llvm::Module *module);

        static std::unique_ptr<VectorIntrinsicFoldFunction> create(MaximContext *context, llvm::Intrinsic::ID id, std::string name, llvm::Module *module);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext) override;

    private:
        llvm::Intrinsic::ID _id;
    };

}
