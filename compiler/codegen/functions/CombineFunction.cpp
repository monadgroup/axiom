#include "CombineFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

CombineFunction::CombineFunction(MaximContext *context)
    : Function(context, "combine", context->numType(),
               {Parameter(context->numType(), false, false), Parameter(context->numType(), false, false)},
               nullptr, nullptr) {

}

std::unique_ptr<CombineFunction> CombineFunction::create(MaximContext *context) {
    return std::make_unique<CombineFunction>(context);
}

std::unique_ptr<Value> CombineFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                 std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                 llvm::Function *func, llvm::Module *module) {
    auto leftNum = dynamic_cast<Num*>(params[0].get());
    auto rightNum = dynamic_cast<Num*>(params[1].get());
    assert(leftNum);
    assert(rightNum);

    auto newVec = b.CreateShuffleVector(
        leftNum->vec(b), rightNum->vec(b),
        {0, 3},
        "combined"
    );

    auto undefPos = SourcePos(-1, -1);
    return leftNum->withVec(b, newVec, undefPos, undefPos);
}
