#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class DelayFunction : public Function {
    public:
        explicit DelayFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<DelayFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;

        std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) override;

        void sampleArguments(ComposableModuleClassMethod *method, size_t index, const std::vector<std::unique_ptr<Value>> &args, const std::vector<std::unique_ptr<Value>> &varargs) override;

    private:
        llvm::StructType *getChannelType();

        llvm::StructType *getContextType();
    };

}
