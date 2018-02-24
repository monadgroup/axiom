#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class ActiveFunction : public Function {
    public:
        ActiveFunction(MaximContext *context);

        static std::unique_ptr<ActiveFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value>
        generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg,
                 llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;
    };

}
