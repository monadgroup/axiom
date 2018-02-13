#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class CombineFunction : public Function {
    public:
        CombineFunction(MaximContext *context, llvm::Module *module);

        static std::unique_ptr<CombineFunction> create(MaximContext *context, llvm::Module *module);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext) override;
    };

}
