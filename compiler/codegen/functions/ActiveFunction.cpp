#include "ActiveFunction.h"

#include <llvm/IR/Constants.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

ActiveFunction::ActiveFunction(MaximContext *context)
    : Function(context, "active", context->numType(), {Parameter(context->numType(), false, false)}, nullptr, nullptr) {

}

std::unique_ptr<ActiveFunction> ActiveFunction::create(MaximContext *context) {
    return std::make_unique<ActiveFunction>(context);
}

std::unique_ptr<Value> ActiveFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                llvm::Module *module) {
    auto xNum = dynamic_cast<Num*>(params[0].get());
    assert(xNum);

    auto activeFloat = b.CreateUIToFP(xNum->active(b), llvm::Type::getFloatTy(context()->llvm()), "active.float");
    auto activeVec = b.CreateInsertElement(llvm::UndefValue::get(context()->numType()->vecType()), activeFloat, (uint64_t) 0, "active.vec");
    activeVec = b.CreateInsertElement(activeVec, activeFloat, (uint64_t) 1, "active.vec");

    auto undefPos = SourcePos(-1, -1);
    auto newNum = Num::create(context(), llvm::UndefValue::get(context()->numType()->get()), undefPos, undefPos);
    newNum = newNum->withVec(b, activeVec, undefPos, undefPos);
    newNum = newNum->withForm(b, MaximCommon::FormType::LINEAR, undefPos, undefPos);
    return newNum->withActive(b, true, undefPos, undefPos);
}
