#include "SequenceFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

SequenceFunction::SequenceFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "sequence", ctx->numType(),
               {Parameter(ctx->numType(), false, false)},
               Parameter::create(ctx->numType(), false, false), nullptr) {

}

std::unique_ptr<SequenceFunction> SequenceFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SequenceFunction>(ctx, module);
}

std::unique_ptr<Value> SequenceFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) {
    auto floorFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::floor,
                                                     {ctx()->numType()->vecType()});

    auto indexNum = dynamic_cast<Num *>(params[0].get());
    assert(indexNum);

    auto &b = method->builder();

    auto countSplat = b.CreateVectorSplat(2, vararg->count(b), "countsplat");
    auto countFloat = b.CreateUIToFP(
        countSplat,
        ctx()->numType()->vecType(),
        "countfloat"
    );

    auto indexVec = indexNum->vec(b);
    auto roundedVec = CreateCall(b, floorFunc, {indexVec}, "index.floored");
    auto modVec = b.CreateFRem(
        roundedVec,
        countFloat,
        "index.modded"
    );
    auto indexUI = b.CreateFPToUI(modVec, countSplat->getType(), "index.ui");

    auto leftIndex = b.CreateExtractElement(indexUI, (uint64_t) 0, "index.left");
    auto rightIndex = b.CreateExtractElement(indexUI, (uint64_t) 1, "index.right");

    auto leftVal = vararg->atIndex(leftIndex, b);
    auto rightVal = vararg->atIndex(rightIndex, b);
    auto leftNum = dynamic_cast<Num *>(leftVal.get());
    auto rightNum = dynamic_cast<Num *>(rightVal.get());
    assert(leftNum && rightNum);

    auto resultVec = b.CreateShuffleVector(
        leftNum->vec(b), rightNum->vec(b),
        {0, 3},
        "asd"
    );
    auto active = b.CreateOr(leftNum->active(b), rightNum->active(b), "active");
    active = b.CreateOr(active, indexNum->active(b), "active");

    auto firstVal = vararg->atIndex((uint64_t) 0, b);
    auto firstNum = dynamic_cast<Num *>(firstVal.get());
    assert(firstNum);

    auto undefPos = SourcePos(-1, -1);
    return firstNum->withVec(b, resultVec, undefPos, undefPos)->withActive(b, active, undefPos, undefPos);
}
