#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class ActiveFunction : public Function {
    public:
        ActiveFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<ActiveFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;
    };

}
