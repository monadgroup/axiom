#include "PanFunction.h"

#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instruction.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

PanFunction::PanFunction(MaximContext *context)
    : Function(context, "pan", context->numType(),
               {Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false)},
               nullptr, nullptr) {

}

std::unique_ptr<PanFunction> PanFunction::create(MaximContext *context) {
    return std::make_unique<PanFunction>(context);
}

std::unique_ptr<Value> PanFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                             std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                             llvm::Function *func, llvm::Module *module) {
    auto minIntrinsic = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::minnum, {context()->numType()->vecType()});
    auto maxIntrinsic = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::maxnum, {context()->numType()->vecType()});

    auto xNum = dynamic_cast<Num*>(params[0].get());
    auto panNum = dynamic_cast<Num*>(params[1].get());
    assert(xNum);
    assert(panNum);

    auto zeroFloat = context()->constFloat(0);
    auto oneFloat = context()->constFloat(1);

    auto panVec = panNum->vec(b);

    // left amplitude = 1 - pan
    auto leftAmplitude = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FSub,
        oneFloat,
        b.CreateExtractElement(panVec, (uint64_t) 0, "pan.left"),
        "amplitude.left"
    );

    // right amplitude = 1 + pan
    auto rightAmplitude = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FAdd,
        oneFloat,
        b.CreateExtractElement(panVec, (uint64_t) 1, "pan.right"),
        "amplitude.right"
    );

    // put values back into vector
    auto amplitudeVec = b.CreateInsertElement(llvm::UndefValue::get(context()->numType()->vecType()), leftAmplitude, (uint64_t) 0, "amplitude");
    amplitudeVec = b.CreateInsertElement(amplitudeVec, rightAmplitude, (uint64_t) 1, "amplitude");

    // clamp to [0, 1] range
    amplitudeVec = CreateCall(
        b, maxIntrinsic,
        {amplitudeVec, llvm::ConstantVector::get({zeroFloat, zeroFloat})},
        "amplitude.maxed"
    );
    amplitudeVec = CreateCall(
        b, minIntrinsic,
        {amplitudeVec, llvm::ConstantVector::get({oneFloat, oneFloat})},
        "amplitude.clamped"
    );

    // multiply amplitude by input value
    auto resultVec = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FMul,
        xNum->vec(b),
        amplitudeVec,
        "panned"
    );

    auto undefPos = SourcePos(-1, -1);
    return xNum->withVec(b, resultVec, undefPos, undefPos);
}
