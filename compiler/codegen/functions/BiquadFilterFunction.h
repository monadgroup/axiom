#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class BiquadFilterFunction : public Function {
    public:
        explicit BiquadFilterFunction(MaximContext *ctx, llvm::Module *module, Function *biquadFilterFunction, const std::string &name);

    protected:
        std::unique_ptr<Value> generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;

        virtual void generateCoefficients(llvm::IRBuilder<> &builder, llvm::Value *q, llvm::Value *k, llvm::Value *kSquared, llvm::Value *a0Ptr, llvm::Value *a1Ptr, llvm::Value *a2Ptr, llvm::Value *b1Ptr, llvm::Value *b2Ptr) = 0;

    private:
        Function *biquadFilterFunction;
    };

}
