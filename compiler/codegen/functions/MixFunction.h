#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class MixFunction : public Function {
    public:
        explicit MixFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<MixFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;
    };

}
