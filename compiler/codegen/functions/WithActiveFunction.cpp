#include "WithActiveFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

WithActiveFunction::WithActiveFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "withActive", ctx->numType(),
               {Parameter(ctx->numType(), false),
                Parameter(ctx->numType(), false)}, nullptr) {

}

std::unique_ptr<WithActiveFunction> WithActiveFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<WithActiveFunction>(ctx, module);
}

std::unique_ptr<Value>
WithActiveFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                             std::unique_ptr<VarArg> vararg) {
    auto xNum = dynamic_cast<Num *>(params[0].get());
    auto activeNum = dynamic_cast<Num *>(params[1].get());
    assert(xNum && activeNum);

    auto &b = method->builder();

    auto activeVec = activeNum->vec(b);
    auto activeVal = b.CreateFCmp(
        llvm::CmpInst::Predicate::FCMP_ONE,
        b.CreateExtractElement(activeVec, (uint64_t) 0, "active.left"),
        ctx()->constFloat(0),
        "active.notzero"
    );

    auto newNum = Num::create(ctx(), xNum->get(), b, method->allocaBuilder());
    newNum->setActive(b, activeVal);
    return std::move(newNum);
}
