#pragma once

#include <llvm/IR/Intrinsics.h>

#include "../Function.h"

namespace MaximCodegen {

    class VectorIntrinsicFunction : public Function {
    public:
        VectorIntrinsicFunction(MaximContext *context, llvm::Intrinsic::ID id, std::string name, size_t paramCount);

        static std::unique_ptr<VectorIntrinsicFunction> create(MaximContext *context, llvm::Intrinsic::ID id, std::string name, size_t paramCount);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *context, llvm::Function *func, llvm::Module *module) override;

    private:
        llvm::Intrinsic::ID id;
    };

}
