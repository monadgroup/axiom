#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class ChannelFunction : public Function {
    public:
        explicit ChannelFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<ChannelFunction> create(MaximContext *context, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;
    };

}

