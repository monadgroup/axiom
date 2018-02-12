#include "ScalarExternalFunction.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

ScalarExternalFunction::ScalarExternalFunction(MaximContext *context, std::string externalName, std::string name,
                                               size_t paramCount, bool propagateForm, llvm::Module *module)
    : Function(context, std::move(name), context->numType(), std::vector<Parameter>(paramCount, Parameter(context->numType(), false, false)), nullptr, nullptr, module),
      externalName(externalName), propagateForm(propagateForm) {

}

std::unique_ptr<ScalarExternalFunction> ScalarExternalFunction::create(MaximContext *context, std::string externalName,
                                                                       std::string name, size_t paramCount,
                                                                       bool propagateForm, llvm::Module *module) {
    return std::make_unique<ScalarExternalFunction>(context, externalName, name, paramCount, propagateForm, module);
}

std::unique_ptr<Value> ScalarExternalFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                        std::unique_ptr<VarArg> vararg, llvm::Value *funcContext) {
    auto floatTy = llvm::Type::getFloatTy(context()->llvm());
    auto external = llvm::Function::Create(
        llvm::FunctionType::get(floatTy, std::vector<llvm::Type*>(params.size(), floatTy), false),
        llvm::Function::ExternalLinkage,
        externalName, module()
    );

    llvm::Value *isActive = context()->constInt(1, 0, false);

    std::vector<llvm::Value*> llVecs;
    llVecs.reserve(params.size());

    Num *firstParam = nullptr;
    for (const auto &param : params) {
        auto numParam = dynamic_cast<Num*>(param.get());
        assert(numParam);
        if (!firstParam) firstParam = numParam;
        llVecs.push_back(numParam->vec(b));
        isActive = b.CreateOr(isActive, numParam->active(b), "activecheck");
    }
    assert(firstParam);

    auto numElements = context()->numType()->vecType()->getVectorNumElements();
    llvm::Value *res = llvm::UndefValue::get(context()->numType()->vecType());
    for (size_t i = 0; i < numElements; i++) {
        std::vector<llvm::Value*> llValues;
        llValues.reserve(llVecs.size());
        for (auto &llVec : llVecs) {
            llValues.push_back(b.CreateExtractElement(llVec, i, "vec.temp"));
        }

        auto singleResult = b.CreateCall(external, llValues, "result.temp");
        res = b.CreateInsertElement(res, singleResult, i, "result");
    }

    SourcePos undefPos(-1, -1);
    auto num = firstParam->withVec(b, res, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
    if (propagateForm) {
        return std::move(num);
    } else {
        return num->withForm(b, MaximCommon::FormType::LINEAR, undefPos, undefPos);
    }
}
