#include "SequenceFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"
#include "../../util.h"

using namespace MaximCodegen;

SequenceFunction::SequenceFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "sequence", ctx->numType(),
               {Parameter(ctx->numType(), false, false)},
               Parameter::create(ctx->numType(), false, false)) {

}

std::unique_ptr<SequenceFunction> SequenceFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SequenceFunction>(ctx, module);
}

std::unique_ptr<Value>
SequenceFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                           std::unique_ptr<VarArg> vararg) {
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

    auto firstNum = AxiomUtil::strict_unique_cast<Num>(vararg->atIndex((uint64_t) 0, b));
    firstNum->setVec(b, resultVec);
    firstNum->setActive(b, active);
    return std::move(firstNum);
}
