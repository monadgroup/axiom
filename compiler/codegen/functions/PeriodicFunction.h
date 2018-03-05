#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class PeriodicFunction : public Function {
    public:
        explicit PeriodicFunction(MaximContext *ctx, llvm::Module *module, std::string name);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;

        std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) override;

        virtual llvm::Value *nextValue(ComposableModuleClassMethod *method, llvm::Value *period) = 0;
    };

}
