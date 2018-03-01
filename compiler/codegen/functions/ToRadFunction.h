#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class ToRadFunction : public Function {
    public:
        ToRadFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<ToRadFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;
    };

}
