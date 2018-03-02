#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class SequenceFunction : public Function {
    public:
        explicit SequenceFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<SequenceFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;
    };

}
