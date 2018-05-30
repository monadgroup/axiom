#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class BiquadFilterFunction : public Function {
    public:
        explicit BiquadFilterFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<BiquadFilterFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value> generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;
    };

}
