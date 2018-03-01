#include "ActiveFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

ActiveFunction::ActiveFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "active", ctx->numType(), {Parameter(ctx->numType(), false, false)}, nullptr) {

}

std::unique_ptr<ActiveFunction> ActiveFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<ActiveFunction>(ctx, module);
}

std::unique_ptr<Value> ActiveFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) {
    auto xNum = dynamic_cast<Num *>(params[0].get());
    assert(xNum);

    auto &b = method->builder();

    auto activeFloat = b.CreateUIToFP(xNum->active(b), llvm::Type::getFloatTy(ctx()->llvm()), "active.float");
    auto activeVec = b.CreateInsertElement(llvm::UndefValue::get(ctx()->numType()->vecType()), activeFloat,
                                           (uint64_t) 0, "active.vec");
    activeVec = b.CreateInsertElement(activeVec, activeFloat, (uint64_t) 1, "active.vec");

    auto undefPos = SourcePos(-1, -1);
    auto newNum = Num::create(ctx(), llvm::UndefValue::get(ctx()->numType()->get()), undefPos, undefPos);
    newNum = newNum->withVec(b, activeVec, undefPos, undefPos);
    newNum = newNum->withForm(b, MaximCommon::FormType::LINEAR, undefPos, undefPos);
    return newNum->withActive(b, true, undefPos, undefPos);
}
