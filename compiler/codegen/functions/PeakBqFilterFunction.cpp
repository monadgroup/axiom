#include "PeakBqFilterFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

PeakBqFilterFunction::PeakBqFilterFunction(MaximCodegen::MaximContext *ctx, llvm::Module *module,
                                           MaximCodegen::Function *biquadFilterFunction)
    : BiquadFilterFunction(ctx, module, biquadFilterFunction, "peakBqFilter", true) {
}

std::unique_ptr<PeakBqFilterFunction> PeakBqFilterFunction::create(MaximCodegen::MaximContext *ctx,
                                                                   llvm::Module *module,
                                                                   MaximCodegen::Function *biquadFilterFunction) {
    return std::make_unique<PeakBqFilterFunction>(ctx, module, biquadFilterFunction);
}

void PeakBqFilterFunction::generateCoefficients(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *q,
                                                llvm::Value *k, llvm::Value *kSquared, llvm::Value *gain,
                                                llvm::Value *a0Ptr, llvm::Value *a1Ptr, llvm::Value *a2Ptr,
                                                llvm::Value *b1Ptr, llvm::Value *b2Ptr) {
    auto powFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::pow, ctx()->floatVecTy());
    auto absFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::fabs, ctx()->floatVecTy());

    auto &b = method->builder();
    auto v = b.CreateCall(powFunc, {
        ctx()->constFloatVec(10),
        b.CreateFDiv(
            b.CreateCall(absFunc, {gain}),
            ctx()->constFloatVec(20)
        )
    });

    auto hasPositiveGainVec = b.CreateFCmpOGE(gain, ctx()->constFloatVec(0));
    auto hasPositiveGain = b.CreateOr(b.CreateExtractElement(hasPositiveGainVec, (uint64_t) 0), b.CreateExtractElement(hasPositiveGainVec, 1));

    auto positiveGainBlock = llvm::BasicBlock::Create(ctx()->llvm(), "positivegain", method->get(method->moduleClass()->module()));
    auto negativeGainBlock = llvm::BasicBlock::Create(ctx()->llvm(), "negativegain", method->get(method->moduleClass()->module()));
    auto continueBlock = llvm::BasicBlock::Create(ctx()->llvm(), "continue", method->get(method->moduleClass()->module()));
    b.CreateCondBr(hasPositiveGain, positiveGainBlock, negativeGainBlock);
    b.SetInsertPoint(positiveGainBlock);

    {
        // norm = 1 / (1 + 1 / q * K + K * K)
        auto norm = b.CreateFDiv(
            ctx()->constFloatVec(1),
            b.CreateFAdd(
                b.CreateFAdd(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(ctx()->constFloatVec(1), q),
                        k
                    )
                ),
                kSquared
            )
        );

        // a0 = (1 + V / q * K + K * K) * norm
        auto a0 = b.CreateFMul(
            b.CreateFAdd(
                b.CreateFAdd(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(v, q),
                        k
                    )
                ),
                kSquared
            ),
            norm
        );
        b.CreateStore(a0, a0Ptr);

        // a1 = 2 * (K * K - 1) * norm
        auto a1 = b.CreateFMul(
            b.CreateFMul(
                ctx()->constFloatVec(2),
                b.CreateFSub(kSquared, ctx()->constFloatVec(1))
            ),
            norm
        );
        b.CreateStore(a1, a1Ptr);

        // a2 = (1 - V / q * K + K * K) * norm
        auto a2 = b.CreateFMul(
            b.CreateFAdd(
                b.CreateFSub(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(v, q),
                        k
                    )
                ),
                kSquared
            ),
            norm
        );
        b.CreateStore(a2, a2Ptr);

        // b1 = a1
        auto b1 = a1;
        b.CreateStore(b1, b1Ptr);

        // b2 = (1 - 1 / q * K + K * K) * norm
        auto b2 = b.CreateFMul(
            b.CreateFAdd(
                b.CreateFSub(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(ctx()->constFloatVec(1), q),
                        k
                    )
                ),
                kSquared
            ),
            norm
        );
        b.CreateStore(b2, b2Ptr);
        b.CreateBr(continueBlock);
    }

    b.SetInsertPoint(negativeGainBlock);

    {
        // norm = 1 / (1 + V / q * K + K * K)
        auto norm = b.CreateFDiv(
            ctx()->constFloatVec(1),
            b.CreateFAdd(
                b.CreateFAdd(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(v, q),
                        k
                    )
                ),
                kSquared
            )
        );

        // a0 = (1 + 1 / q * K + K * K) * norm
        auto a0 = b.CreateFMul(
            b.CreateFAdd(
                b.CreateFAdd(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(ctx()->constFloatVec(1), q),
                        k
                    )
                ),
                kSquared
            ),
            norm
        );
        b.CreateStore(a0, a0Ptr);

        // a1 = 2 * (K * K - 1) * norm
        auto a1 = b.CreateFMul(
            b.CreateFMul(
                ctx()->constFloatVec(2),
                b.CreateFSub(kSquared, ctx()->constFloatVec(1))
            ),
            norm
        );
        b.CreateStore(a1, a1Ptr);

        // a2 = (1 - 1 / q * K + K * K) * norm
        auto a2 = b.CreateFMul(
            b.CreateFAdd(
                b.CreateFSub(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(ctx()->constFloatVec(1), q),
                        k
                    )
                ),
                kSquared
            ),
            norm
        );
        b.CreateStore(a2, a2Ptr);

        // b1 = a1
        auto b1 = a1;
        b.CreateStore(b1, b1Ptr);

        // b2 = (1 - V / q * K + K * K) * norm
        auto b2 = b.CreateFMul(
            b.CreateFAdd(
                b.CreateFSub(
                    ctx()->constFloatVec(1),
                    b.CreateFMul(
                        b.CreateFDiv(v, q),
                        k
                    )
                ),
                kSquared
            ),
            norm
        );
        b.CreateStore(b2, b2Ptr);
        b.CreateBr(continueBlock);
    }

    b.SetInsertPoint(continueBlock);
}
