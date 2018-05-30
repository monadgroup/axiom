#include "BiquadFilterFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

BiquadFilterFunction::BiquadFilterFunction(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "biquadFilter",
               ctx->numType(),
               {Parameter(ctx->numType(), false, false), // input
                Parameter(ctx->numType(), false, false), // a0
                Parameter(ctx->numType(), false, false), // a1
                Parameter(ctx->numType(), false, false), // a2
                Parameter(ctx->numType(), false, false), // b1
                Parameter(ctx->numType(), false, false)  // b2
               }, nullptr) {
}

std::unique_ptr<BiquadFilterFunction> BiquadFilterFunction::create(MaximCodegen::MaximContext *ctx,
                                                                   llvm::Module *module) {
    return std::make_unique<BiquadFilterFunction>(ctx, module);
}

std::unique_ptr<Value> BiquadFilterFunction::generate(MaximCodegen::ComposableModuleClassMethod *method,
                                                      const std::vector<std::unique_ptr<MaximCodegen::Value>> &params,
                                                      std::unique_ptr<MaximCodegen::VarArg> vararg) {
    auto inputNum = dynamic_cast<Num *>(params[0].get());
    auto a0Num = dynamic_cast<Num *>(params[1].get());
    auto a1Num = dynamic_cast<Num *>(params[2].get());
    auto a2Num = dynamic_cast<Num *>(params[3].get());
    auto b1Num = dynamic_cast<Num *>(params[4].get());
    auto b2Num = dynamic_cast<Num *>(params[5].get());
    assert(inputNum && a0Num && a1Num && a2Num && b1Num && b2Num);

    auto &b = method->builder();
    auto z1Ptr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->floatVecTy()), "z1.ptr");
    auto z2Ptr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->floatVecTy()), "z2.ptr");

    auto inputVec = inputNum->vec(b);
    auto outVec = b.CreateFAdd(
        b.CreateFMul(inputVec, a0Num->vec(b)),
        b.CreateLoad(z1Ptr)
    );
    auto newZ1 = b.CreateFSub(
        b.CreateFAdd(
            b.CreateFMul(inputVec, a1Num->vec(b)),
            b.CreateLoad(z2Ptr)
        ),
        b.CreateFMul(
            b1Num->vec(b),
            outVec
        )
    );
    b.CreateStore(newZ1, z1Ptr);

    auto newZ2 = b.CreateFSub(
        b.CreateFMul(
            inputVec,
            a2Num->vec(b)
        ),
        b.CreateFMul(
            b2Num->vec(b),
            outVec
        )
    );
    b.CreateStore(newZ2, z2Ptr);

    auto resultNum = Num::create(ctx(), inputNum->get(), b, method->allocaBuilder());
    resultNum->setVec(b, outVec);
    return std::move(resultNum);
}
