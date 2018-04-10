#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class HoldFunction : public Function {
    public:
        explicit HoldFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<HoldFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;

        std::vector<std::unique_ptr<Value>>
        mapArguments(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> providedArgs) override;
    };

}
