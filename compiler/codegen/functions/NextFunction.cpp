#include "NextFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

NextFunction::NextFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "next", ctx->numType(), {Parameter(ctx->numType(), false, false)},
               nullptr) {

}

std::unique_ptr<NextFunction> NextFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<NextFunction>(ctx, module);
}

std::unique_ptr<Value>
NextFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                       std::unique_ptr<VarArg> vararg) {
    auto paramVal = dynamic_cast<Num *>(params[0].get());
    assert(paramVal);

    auto &b = method->builder();
    auto funcContext = method->getEntryPointer(addEntry(ctx()->numType()->get()), "ctx");

    auto returnVal = Num::create(ctx(), funcContext, b, method->allocaBuilder());
    b.CreateStore(b.CreateLoad(paramVal->get(), "param.deref"), funcContext);
    return std::move(returnVal);
}
