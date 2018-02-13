#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class NoiseFunction : public Function {
    public:
        NoiseFunction(MaximContext *context);

        static std::unique_ptr<NoiseFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Module *module) override;

        std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) override;
    };

}
