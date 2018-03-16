#include "MixFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

MixFunction::MixFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "mix", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false)},
               nullptr) {

}

std::unique_ptr<MixFunction> MixFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<MixFunction>(ctx, module);
}

std::unique_ptr<Value>
MixFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                      std::unique_ptr<VarArg> vararg) {
    auto numA = dynamic_cast<Num *>(params[0].get());
    auto numB = dynamic_cast<Num *>(params[1].get());
    auto mixNum = dynamic_cast<Num *>(params[2].get());
    assert(numA && numB && mixNum);

    auto &b = method->builder();

    auto vecA = numA->vec(b);
    auto vecB = numB->vec(b);
    auto mixVec = mixNum->vec(b);

    auto vecDiff = b.CreateFSub(vecB, vecA, "diff");
    auto vecMul = b.CreateFMul(vecDiff, mixVec, "mul");
    auto vecResult = b.CreateFAdd(vecMul, vecA, "result");

    auto active = b.CreateOr(numA->active(b), numB->active(b), "active");

    auto newNum = Num::create(ctx(), numA->get(), b, method->allocaBuilder());
    newNum->setVec(b, vecResult);
    newNum->setActive(b, active);
    return std::move(newNum);
}
