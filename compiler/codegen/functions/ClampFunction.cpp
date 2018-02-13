#include "ClampFunction.h"

#include <llvm/IR/Intrinsics.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

ClampFunction::ClampFunction(MaximContext *context, llvm::Module *module)
    : Function(context, "clamp", context->numType(),
               {Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false)},
               nullptr, nullptr, module) {

}

std::unique_ptr<ClampFunction> ClampFunction::create(MaximContext *context, llvm::Module *module) {
    return std::make_unique<ClampFunction>(context, module);
}

std::unique_ptr<Value> ClampFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                               std::unique_ptr<VarArg> vararg, llvm::Value *funcContext) {
    auto minIntrinsic = llvm::Intrinsic::getDeclaration(module(), llvm::Intrinsic::ID::minnum, {context()->numType()->vecType()});
    auto maxIntrinsic = llvm::Intrinsic::getDeclaration(module(), llvm::Intrinsic::ID::maxnum, {context()->numType()->vecType()});

    auto xNum = dynamic_cast<Num*>(params[0].get());
    auto minNum = dynamic_cast<Num*>(params[1].get());
    auto maxNum = dynamic_cast<Num*>(params[2].get());
    assert(xNum);
    assert(minNum);
    assert(maxNum);

    auto currentVec = xNum->vec(b);
    currentVec = b.CreateCall(maxIntrinsic, {currentVec, minNum->vec(b)}, "maxed");
    currentVec = b.CreateCall(minIntrinsic, {currentVec, maxNum->vec(b)}, "clamped");

    auto undefPos = SourcePos(-1, -1);
    return xNum->withVec(b, currentVec, undefPos, undefPos);
}
