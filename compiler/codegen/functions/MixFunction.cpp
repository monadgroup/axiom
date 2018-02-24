#include "MixFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

MixFunction::MixFunction(MaximContext *context)
    : Function(context, "mix", context->numType(),
               {Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false)},
               nullptr, nullptr) {

}

std::unique_ptr<MixFunction> MixFunction::create(MaximContext *context) {
    return std::make_unique<MixFunction>(context);
}

std::unique_ptr<Value> MixFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                             std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                             llvm::Function *func, llvm::Module *module) {
    auto numA = dynamic_cast<Num *>(params[0].get());
    auto numB = dynamic_cast<Num *>(params[1].get());
    auto mixNum = dynamic_cast<Num *>(params[2].get());
    assert(numA && numB && mixNum);

    auto vecA = numA->vec(b);
    auto vecB = numB->vec(b);
    auto mixVec = mixNum->vec(b);

    auto vecDiff = b.CreateFSub(vecB, vecA, "diff");
    auto vecMul = b.CreateFMul(vecDiff, mixVec, "mul");
    auto vecResult = b.CreateFAdd(vecMul, vecA, "result");

    auto active = b.CreateOr(numA->active(b), numB->active(b), "active");

    auto undefPos = SourcePos(-1, -1);
    return numA->withVec(b, vecResult, undefPos, undefPos)->withActive(b, active, undefPos, undefPos);
}
