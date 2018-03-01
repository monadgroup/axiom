#include <llvm/IR/Constants.h>
#include "VectorIntrinsicFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

VectorIntrinsicFunction::VectorIntrinsicFunction(MaximContext *context, llvm::Intrinsic::ID id, std::string name,
                                                 size_t paramCount)
    : Function(context, std::move(name), context->numType(),
               std::vector<Parameter>(paramCount, Parameter(context->numType(), false, false)), nullptr, nullptr),
      id(id) {

}

std::unique_ptr<VectorIntrinsicFunction> VectorIntrinsicFunction::create(MaximContext *context, llvm::Intrinsic::ID id,
                                                                         std::string name, size_t paramCount) {
    return std::make_unique<VectorIntrinsicFunction>(context, id, name, paramCount);
}

std::unique_ptr<Value> VectorIntrinsicFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                         std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                         llvm::Function *func, llvm::Module *module) {
    auto intrinsic = llvm::Intrinsic::getDeclaration(module, id, {context()->numType()->vecType()});

    llvm::Value *isActive = context()->constInt(1, 0, false);

    std::vector<llvm::Value *> llParams;
    llParams.reserve(params.size());

    Num *firstParam = nullptr;
    for (const auto &param : params) {
        auto numParam = dynamic_cast<Num *>(param.get());
        assert(numParam);
        if (!firstParam) firstParam = numParam;
        llParams.push_back(numParam->vec(b));
        isActive = b.CreateOr(isActive, numParam->active(b), "activecheck");
    }
    assert(firstParam);

    auto res = CreateCall(b, intrinsic, llParams, "intrinsic.result");

    SourcePos undefPos(-1, -1);
    return firstParam->withVec(b, res, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
}
