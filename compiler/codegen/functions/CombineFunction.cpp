#include "CombineFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

CombineFunction::CombineFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "combine", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false)},
               nullptr) {

}

std::unique_ptr<CombineFunction> CombineFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<CombineFunction>(ctx, module);
}

std::unique_ptr<Value>
CombineFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                          std::unique_ptr<VarArg> vararg) {
    auto leftNum = dynamic_cast<Num *>(params[0].get());
    auto rightNum = dynamic_cast<Num *>(params[1].get());
    assert(leftNum && rightNum);

    auto &b = method->builder();

    auto newVec = b.CreateShuffleVector(
        leftNum->vec(b), rightNum->vec(b),
        {0, 3},
        "combined"
    );

    auto newNum = Num::create(ctx(), leftNum->get(), b, method->allocaBuilder());
    newNum->setVec(b, newVec);
    return std::move(newNum);
}
