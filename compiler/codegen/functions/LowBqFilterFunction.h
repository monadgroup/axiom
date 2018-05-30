#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class LowBqFilterFunction : public Function {
    public:
        explicit LowBqFilterFunction(MaximContext *ctx, llvm::Module *module, Function *biquadFilterFunction);

        static std::unique_ptr<LowBqFilterFunction> create(MaximContext *ctx, llvm::Module *module, Function *biquadFilterFunction);

    protected:
        std::unique_ptr<Value> generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;

    private:
        Function *biquadFilterFunction;
    };

}
