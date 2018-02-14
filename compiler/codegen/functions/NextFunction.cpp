#include "NextFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

NextFunction::NextFunction(MaximContext *context)
    : Function(context, "next", context->numType(), {Parameter(context->numType(), false, false)},
               nullptr, context->numType()->get()) {

}

std::unique_ptr<NextFunction> NextFunction::create(MaximContext *context) {
    return std::make_unique<NextFunction>(context);
}

std::unique_ptr<Value> NextFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                              std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                              llvm::Function *func, llvm::Module *module) {
    auto paramVal = dynamic_cast<Num*>(params[0].get());
    assert(paramVal);

    auto undefPos = SourcePos(-1, -1);
    auto returnVal = Num::create(context(), b.CreateLoad(funcContext, "result"), undefPos, undefPos);
    b.CreateStore(paramVal->get(), funcContext);

    return std::move(returnVal);
}

std::unique_ptr<Instantiable> NextFunction::generateCall(std::vector<std::unique_ptr<Value>> args) {
    return std::make_unique<NextFunctionCall>();
}

llvm::Constant* NextFunction::NextFunctionCall::getInitialVal(MaximContext *ctx) {
    return llvm::ConstantStruct::get(ctx->numType()->get(), {
        llvm::ConstantVector::get({ctx->constFloat(0), ctx->constFloat(0)}),
        llvm::ConstantInt::get(ctx->numType()->formType(), (uint64_t) MaximCommon::FormType::LINEAR, false),
        llvm::ConstantInt::get(ctx->numType()->activeType(), (uint64_t) true, false)
    });
}

llvm::Type* NextFunction::NextFunctionCall::type(MaximContext *ctx) const {
    return ctx->numType()->get();
}
