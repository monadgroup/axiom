#include "NoiseFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

NoiseFunction::NoiseFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "noise", ctx->numType(),
               {Parameter(ctx->numType(), false, true),
                Parameter(ctx->numType(), false, true)},
               nullptr) {

}

std::unique_ptr<NoiseFunction> NoiseFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<NoiseFunction>(ctx, module);
}

std::unique_ptr<Value>
NoiseFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                        std::unique_ptr<VarArg> vararg) {
    auto randType = llvm::Type::getInt32Ty(ctx()->llvm());
    auto randFunc = llvm::Function::Create(
        llvm::FunctionType::get(randType, {}, false),
        llvm::Function::ExternalLinkage,
        "rand", method->moduleClass()->module()
    );

    auto &b = method->builder();

    auto minNum = dynamic_cast<Num *>(params[0].get());
    auto maxNum = dynamic_cast<Num *>(params[1].get());
    assert(minNum && maxNum);

    auto minVec = minNum->vec(b);
    auto maxVec = maxNum->vec(b);

    auto magnitude = b.CreateBinOp(llvm::Instruction::BinaryOps::FSub, maxVec, minVec, "magnitude");

    auto leftRand = CreateCall(b, randFunc, {}, "rand.left");
    auto rightRand = CreateCall(b, randFunc, {}, "rand.right");
    auto randVec = b.CreateInsertElement(llvm::UndefValue::get(llvm::VectorType::get(randType, 2)), leftRand,
                                         (uint64_t) 0, "rand");
    randVec = b.CreateInsertElement(randVec, rightRand, (uint64_t) 1, "rand");
    auto randFloatVec = b.CreateSIToFP(randVec, ctx()->numType()->vecType(), "rand.float");

    // todo: probably shouldn't use RAND_MAX here as it's compiler-dependent
    auto randMaxFloat = ctx()->constFloat(RAND_MAX);
    auto randNormalized = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FDiv,
        randFloatVec,
        llvm::ConstantVector::get({randMaxFloat, randMaxFloat}),
        "rand.normalized"
    );
    auto randVal = b.CreateBinOp(llvm::Instruction::BinaryOps::FMul, randNormalized, magnitude, "rand.result");
    randVal = b.CreateBinOp(llvm::Instruction::BinaryOps::FAdd, randVal, minVec, "rand.result");

    auto numResult = Num::create(ctx(), method->allocaBuilder());
    numResult->setVec(b, randVal);
    numResult->setForm(b, MaximCommon::FormType::LINEAR);
    numResult->setActive(b, true);
    return std::move(numResult);
}

std::vector<std::unique_ptr<Value>>
NoiseFunction::mapArguments(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> providedArgs) {
    auto undefPos = SourcePos(-1, -1);
    if (providedArgs.empty()) {
        providedArgs.push_back(
            Num::create(ctx(), method->allocaBuilder(), -1, -1, MaximCommon::FormType::LINEAR, true, undefPos,
                        undefPos));
    }
    if (providedArgs.size() < 2) {
        providedArgs.push_back(
            Num::create(ctx(), method->allocaBuilder(), 1, 1, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}
