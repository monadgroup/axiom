#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class CombineFunction : public Function {
    public:
        CombineFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<CombineFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;
    };

}
