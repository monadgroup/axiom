#pragma once

#include "../Function.h"
#include "../Instantiable.h"

namespace MaximCodegen {

    class MixFunction : public Function {
    public:
        explicit MixFunction(MaximContext *context);

        static std::unique_ptr<MixFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;
    };

}
