#include "AmplitudeFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

AmplitudeFunction::AmplitudeFunction(MaximContext *context)
    : Function(context, "amplitude", context->numType(), {Parameter(context->numType(), false, false)}, nullptr,
               context->numType()->vecType()) {
    b0 = 1 - std::exp(-1 / (0.05f * context->sampleRate));
}

std::unique_ptr<AmplitudeFunction> AmplitudeFunction::create(MaximContext *context) {
    return std::make_unique<AmplitudeFunction>(context);
}

std::unique_ptr<Value> AmplitudeFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                   std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                   llvm::Function *func, llvm::Module *module) {
    auto absIntrinsic = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::fabs,
                                                        {context()->numType()->vecType()});

    auto paramVal = dynamic_cast<Num *>(params[0].get());
    assert(paramVal);

    auto absInput = CreateCall(b, absIntrinsic, {paramVal->vec(b)}, "inputabs");
    auto currentEstimate = b.CreateLoad(funcContext, "currentest");
    auto inputDiff = b.CreateBinOp(llvm::Instruction::BinaryOps::FSub, absInput, currentEstimate, "inputdiff");
    auto inputMul = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FMul,
        llvm::ConstantVector::getSplat(2, context()->constFloat(b0)),
        inputDiff, "inputmul"
    );
    auto newEstimate = b.CreateBinOp(llvm::Instruction::BinaryOps::FAdd, currentEstimate, inputMul, "newest");
    b.CreateStore(newEstimate, funcContext);

    auto undefPos = SourcePos(-1, -1);
    return paramVal->withVec(b, newEstimate, undefPos, undefPos)->withForm(b, MaximCommon::FormType::LINEAR, undefPos,
                                                                           undefPos);
}

std::unique_ptr<Instantiable> AmplitudeFunction::generateCall(std::vector<std::unique_ptr<Value>> args) {
    return std::make_unique<FunctionCall>();
}

llvm::Constant *AmplitudeFunction::FunctionCall::getInitialVal(MaximContext *ctx) {
    return llvm::ConstantVector::getSplat(2, ctx->constFloat(0));
}

llvm::Type *AmplitudeFunction::FunctionCall::type(MaximContext *ctx) const {
    return ctx->numType()->vecType();
}
