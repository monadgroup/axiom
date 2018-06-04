#include "HighBqFilterFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

HighBqFilterFunction::HighBqFilterFunction(MaximCodegen::MaximContext *ctx, llvm::Module *module,
                                           MaximCodegen::Function *biquadFilterFunction)
    : BiquadFilterFunction(ctx, module, biquadFilterFunction, "highBqFilter", false) {
}

std::unique_ptr<HighBqFilterFunction> HighBqFilterFunction::create(MaximCodegen::MaximContext *ctx, llvm::Module *module,
                                                                  MaximCodegen::Function *biquadFilterFunction) {
    return std::make_unique<HighBqFilterFunction>(ctx, module, biquadFilterFunction);
}

void HighBqFilterFunction::generateCoefficients(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *q,
                                                llvm::Value *k, llvm::Value *kSquared, llvm::Value *gain,
                                                llvm::Value *a0Ptr, llvm::Value *a1Ptr, llvm::Value *a2Ptr,
                                                llvm::Value *b1Ptr, llvm::Value *b2Ptr) {
    auto &b = method->builder();

    // norm = 1 / (1 + K / q + K * K)
    auto norm = b.CreateFDiv(
        ctx()->constFloatVec(1),
        b.CreateFAdd(
            b.CreateFAdd(
                ctx()->constFloatVec(1),
                b.CreateFDiv(k, q)
            ),
            kSquared
        )
    );

    // a0 = norm
    auto a0 = norm;
    b.CreateStore(a0, a0Ptr);

    // a1 = -2 * a0
    auto a1 = b.CreateFMul(ctx()->constFloatVec(-2), a0);
    b.CreateStore(a1, a1Ptr);

    // a2 = a0
    auto a2 = a0;
    b.CreateStore(a2, a2Ptr);

    // b1 = 2 * (K * K - 1) * norm
    auto b1 = b.CreateFMul(
        b.CreateFMul(
            ctx()->constFloatVec(2),
            b.CreateFSub(kSquared, ctx()->constFloatVec(1))
        ),
        norm
    );
    b.CreateStore(b1, b1Ptr);

    // b2 = (1 - K / q + K * K) * norm
    auto b2 = b.CreateFMul(
        b.CreateFAdd(
            b.CreateFSub(
                ctx()->constFloatVec(1),
                b.CreateFDiv(k, q)
            ),
            kSquared
        ),
        norm
    );
    b.CreateStore(b2, b2Ptr);
}
