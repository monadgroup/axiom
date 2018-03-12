#include "PanFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

PanFunction::PanFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "pan", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false)},
               nullptr) {

}

std::unique_ptr<PanFunction> PanFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<PanFunction>(ctx, module);
}

std::unique_ptr<Value>
PanFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                      std::unique_ptr<VarArg> vararg) {
    auto minIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::minnum,
                                                        {ctx()->numType()->vecType()});
    auto maxIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::maxnum,
                                                        {ctx()->numType()->vecType()});

    auto xNum = dynamic_cast<Num *>(params[0].get());
    auto panNum = dynamic_cast<Num *>(params[1].get());
    assert(xNum && panNum);

    auto &b = method->builder();

    auto panVec = panNum->vec(b);

    // todo: use sqr() instead of this method for proper values

    // left amplitude = 1 - pan
    auto leftAmplitude = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FSub,
        ctx()->constFloat(1),
        b.CreateExtractElement(panVec, (uint64_t) 0, "pan.left"),
        "amplitude.left"
    );

    // right amplitude = 1 + pan
    auto rightAmplitude = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FAdd,
        ctx()->constFloat(1),
        b.CreateExtractElement(panVec, (uint64_t) 1, "pan.right"),
        "amplitude.right"
    );

    // put values back into vector
    auto amplitudeVec = b.CreateInsertElement(llvm::UndefValue::get(ctx()->numType()->vecType()), leftAmplitude,
                                              (uint64_t) 0, "amplitude");
    amplitudeVec = b.CreateInsertElement(amplitudeVec, rightAmplitude, (uint64_t) 1, "amplitude");

    // clamp to [0, 1] range
    amplitudeVec = CreateCall(
        b, maxIntrinsic,
        {amplitudeVec, ctx()->constFloatVec(0)},
        "amplitude.maxed"
    );
    amplitudeVec = CreateCall(
        b, minIntrinsic,
        {amplitudeVec, ctx()->constFloatVec(1)},
        "amplitude.clamped"
    );

    // multiply amplitude by input value
    auto resultVec = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FMul,
        xNum->vec(b),
        amplitudeVec,
        "panned"
    );

    xNum->setVec(b, resultVec);
    return xNum->clone();
}
