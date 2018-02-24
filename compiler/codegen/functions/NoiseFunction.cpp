#include "NoiseFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

NoiseFunction::NoiseFunction(MaximContext *context)
    : Function(context, "noise", context->numType(),
               {Parameter(context->numType(), false, true), Parameter(context->numType(), false, true)},
               nullptr, nullptr, false) {

}

std::unique_ptr<NoiseFunction> NoiseFunction::create(MaximContext *context) {
    return std::make_unique<NoiseFunction>(context);
}

std::unique_ptr<Value> NoiseFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                               std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                               llvm::Function *func, llvm::Module *module) {
    auto randType = llvm::Type::getInt32Ty(context()->llvm());
    auto randFunc = llvm::Function::Create(
        llvm::FunctionType::get(randType, {}, false),
        llvm::Function::ExternalLinkage,
        "rand", module
    );

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
    auto randFloatVec = b.CreateSIToFP(randVec, context()->numType()->vecType(), "rand.float");

    // todo: probably shouldn't use RAND_MAX here as it's compiler-dependent
    auto randMaxFloat = context()->constFloat(RAND_MAX);
    auto randNormalized = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FDiv,
        randFloatVec,
        llvm::ConstantVector::get({randMaxFloat, randMaxFloat}),
        "rand.normalized"
    );
    auto randVal = b.CreateBinOp(llvm::Instruction::BinaryOps::FMul, randNormalized, magnitude, "rand.result");
    randVal = b.CreateBinOp(llvm::Instruction::BinaryOps::FAdd, randVal, minVec, "rand.result");

    auto undefPos = SourcePos(-1, -1);
    auto numResult = Num::create(context(), llvm::UndefValue::get(context()->numType()->get()), undefPos, undefPos);
    numResult = numResult->withVec(b, randVal, undefPos, undefPos);
    numResult = numResult->withForm(b, MaximCommon::FormType::LINEAR, undefPos, undefPos);
    return numResult->withActive(b, true, undefPos, undefPos);
}

std::vector<std::unique_ptr<Value>> NoiseFunction::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    auto undefPos = SourcePos(-1, -1);
    if (providedArgs.empty()) {
        providedArgs.push_back(Num::create(context(), -1, -1, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    if (providedArgs.size() < 2) {
        providedArgs.push_back(Num::create(context(), 1, 1, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}
