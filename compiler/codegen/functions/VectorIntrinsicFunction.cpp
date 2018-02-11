#include <llvm/IR/Constants.h>
#include "VectorIntrinsicFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

VectorIntrinsicFunction::VectorIntrinsicFunction(MaximContext *context, llvm::Intrinsic::ID id, std::string name,
                                                 size_t paramCount, bool propagateForm, llvm::Module *module)
    : Function(context, std::move(name), context->numType(), std::vector<Parameter>(paramCount, Parameter(context->numType(), false, false)), nullptr, nullptr, module),
      id(id), propagateForm(propagateForm) {

}

std::unique_ptr<Value> VectorIntrinsicFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                         std::unique_ptr<VarArg> vararg, llvm::Value *funcContext) {
    auto intrinsic = llvm::Intrinsic::getDeclaration(module(), id, {context()->numType()->get()});
    std::vector<llvm::Value*> llParams;
    llParams.reserve(params.size());

    Num *firstParam = nullptr;
    for (const auto &param : params) {
        auto numParam = dynamic_cast<Num*>(param.get());
        assert(numParam);
        if (!firstParam) firstParam = numParam;
        llParams.push_back(numParam->vec(b));
    }
    assert(firstParam);

    auto res = b.CreateCall(intrinsic, llParams, "intrinsic.result");

    SourcePos undefPos(-1, -1);
    auto num = firstParam->withVec(b, res, undefPos, undefPos);
    if (propagateForm) {
        return std::move(num);
    } else {
        return num->withForm(b, MaximCommon::FormType::LINEAR, undefPos, undefPos);
    }
}
