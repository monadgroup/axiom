#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class VoicesFunction : public Function {
    public:
        explicit VoicesFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<VoicesFunction> create(MaximContext *context, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;
    };

}
