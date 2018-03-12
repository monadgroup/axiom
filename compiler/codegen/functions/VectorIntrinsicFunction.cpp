#include "VectorIntrinsicFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

VectorIntrinsicFunction::VectorIntrinsicFunction(MaximContext *ctx, llvm::Module *module, llvm::Intrinsic::ID id,
                                                 std::string name, size_t paramCount)
    : Function(ctx, module, std::move(name), ctx->numType(),
               std::vector<Parameter>(paramCount, Parameter(ctx->numType(), false)), nullptr),
      id(id) {

}

std::unique_ptr<VectorIntrinsicFunction> VectorIntrinsicFunction::create(MaximContext *ctx, llvm::Module *module,
                                                                         llvm::Intrinsic::ID id, std::string name,
                                                                         size_t paramCount) {
    return std::make_unique<VectorIntrinsicFunction>(ctx, module, id, name, paramCount);
}

std::unique_ptr<Value> VectorIntrinsicFunction::generate(ComposableModuleClassMethod *method,
                                                         const std::vector<std::unique_ptr<Value>> &params,
                                                         std::unique_ptr<VarArg> vararg) {
    auto intrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), id,
                                                     {ctx()->numType()->vecType()});
    auto &b = method->builder();

    llvm::Value *isActive = ctx()->constInt(1, 0, false);

    std::vector<llvm::Value *> llParams;
    llParams.reserve(params.size());

    Num *firstParam = nullptr;
    for (const auto &param : params) {
        auto numParam = dynamic_cast<Num *>(param.get());
        assert(numParam);
        if (!firstParam) firstParam = numParam;
        llParams.push_back(numParam->vec(b));
        isActive = b.CreateOr(isActive, numParam->active(b), "active");
    }
    assert(firstParam);

    auto res = CreateCall(b, intrinsic, llParams, "intrinsic.result");

    auto newNum = Num::create(ctx(), firstParam->get(), b, method->allocaBuilder());
    newNum->setVec(b, res);
    newNum->setActive(b, isActive);
    return std::move(newNum);
}
