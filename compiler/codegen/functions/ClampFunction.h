#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class ClampFunction : public Function {
    public:
        ClampFunction(MaximContext *context, llvm::Module *module);

        static std::unique_ptr<ClampFunction> create(MaximContext *context, llvm::Module *module);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext) override;
    };

}
