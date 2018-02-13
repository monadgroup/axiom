#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class CombineFunction : public Function {
    public:
        CombineFunction(MaximContext *context);

        static std::unique_ptr<CombineFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;
    };

}
