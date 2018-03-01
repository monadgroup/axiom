#include "ClampFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

ClampFunction::ClampFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "clamp", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false)},
               nullptr) {

}

std::unique_ptr<ClampFunction> ClampFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<ClampFunction>(ctx, module);
}

std::unique_ptr<Value> ClampFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) {
    auto minIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::minnum,
                                                        {ctx()->numType()->vecType()});
    auto maxIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::maxnum,
                                                        {ctx()->numType()->vecType()});

    auto &b = method->builder();

    auto xNum = dynamic_cast<Num *>(params[0].get());
    auto minNum = dynamic_cast<Num *>(params[1].get());
    auto maxNum = dynamic_cast<Num *>(params[2].get());
    assert(xNum && minNum && maxNum);

    auto currentVec = xNum->vec(b);
    currentVec = CreateCall(b, maxIntrinsic, {currentVec, minNum->vec(b)}, "maxed");
    currentVec = CreateCall(b, minIntrinsic, {currentVec, maxNum->vec(b)}, "clamped");

    auto undefPos = SourcePos(-1, -1);
    return xNum->withVec(b, currentVec, undefPos, undefPos);
}
