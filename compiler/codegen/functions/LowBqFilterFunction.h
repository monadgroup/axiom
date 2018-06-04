#pragma once

#include "BiquadFilterFunction.h"

namespace MaximCodegen {

    class LowBqFilterFunction : public BiquadFilterFunction {
    public:
        explicit LowBqFilterFunction(MaximContext *ctx, llvm::Module *module, Function *biquadFilterFunction);

        static std::unique_ptr<LowBqFilterFunction> create(MaximContext *ctx, llvm::Module *module, Function *biquadFilterFunction);

    protected:
        void generateCoefficients(ComposableModuleClassMethod *method, llvm::Value *q, llvm::Value *k, llvm::Value *kSquared, llvm::Value *gain, llvm::Value *a0Ptr, llvm::Value *a1Ptr, llvm::Value *a2Ptr, llvm::Value *b1Ptr, llvm::Value *b2Ptr) override;
    };

}
