#include "ActiveFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

ActiveFunction::ActiveFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "active", ctx->numType(), {Parameter(ctx->numType(), false)}, nullptr) {

}

std::unique_ptr<ActiveFunction> ActiveFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<ActiveFunction>(ctx, module);
}

std::unique_ptr<Value>
ActiveFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                         std::unique_ptr<VarArg> vararg) {
    auto xNum = dynamic_cast<Num *>(params[0].get());
    assert(xNum);

    auto &b = method->builder();

    auto activeFloat = b.CreateUIToFP(xNum->active(b), llvm::Type::getFloatTy(ctx()->llvm()), "active.float");
    auto activeVec = b.CreateInsertElement(llvm::UndefValue::get(ctx()->numType()->vecType()), activeFloat,
                                           (uint64_t) 0, "active.vec");
    activeVec = b.CreateInsertElement(activeVec, activeFloat, (uint64_t) 1, "active.vec");

    auto newNum = Num::create(ctx(), method->allocaBuilder());
    newNum->setVec(b, activeVec);
    newNum->setForm(b, MaximCommon::FormType::LINEAR);
    newNum->setActive(b, true);
    return std::move(newNum);
}
