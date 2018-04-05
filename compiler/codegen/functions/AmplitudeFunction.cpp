#include "AmplitudeFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

AmplitudeFunction::AmplitudeFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "amplitude", ctx->numType(), {Parameter(ctx->numType(), false, false)}, nullptr) {
    b0 = 1 - std::exp(-1 / (0.05f * ctx->sampleRate));
}

std::unique_ptr<AmplitudeFunction> AmplitudeFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<AmplitudeFunction>(ctx, module);
}

std::unique_ptr<Value>
AmplitudeFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                            std::unique_ptr<VarArg> vararg) {
    auto absIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::fabs,
                                                        {ctx()->numType()->vecType()});

    auto &b = method->builder();
    auto funcContext = method->getEntryPointer(addEntry(ctx()->floatVecTy()), "ctx");

    auto paramVal = dynamic_cast<Num *>(params[0].get());
    assert(paramVal);

    auto absInput = CreateCall(b, absIntrinsic, {paramVal->vec(b)}, "inputabs");
    auto currentEstimate = b.CreateLoad(funcContext, "currentest");
    auto inputDiff = b.CreateBinOp(llvm::Instruction::BinaryOps::FSub, absInput, currentEstimate, "inputdiff");
    auto inputMul = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FMul,
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(b0)),
        inputDiff, "inputmul"
    );
    auto newEstimate = b.CreateBinOp(llvm::Instruction::BinaryOps::FAdd, currentEstimate, inputMul, "newest");
    b.CreateStore(newEstimate, funcContext);

    auto newNum = Num::create(ctx(), paramVal->get(), b, method->allocaBuilder());
    newNum->setVec(b, newEstimate);
    newNum->setForm(b, MaximCommon::FormType::LINEAR);
    return std::move(newNum);
}
