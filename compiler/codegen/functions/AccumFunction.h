#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class AccumFunction : public Function {
    public:
        explicit AccumFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<AccumFunction> create(MaximContext *context, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;

        std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) override;
    };

}
