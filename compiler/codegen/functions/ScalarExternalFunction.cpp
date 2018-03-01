#include "ScalarExternalFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

ScalarExternalFunction::ScalarExternalFunction(MaximContext *ctx, llvm::Module *module, std::string externalName,
                                               std::string name, size_t paramCount)
    : Function(ctx, module, std::move(name), ctx->numType(), std::vector<Parameter>(paramCount, Parameter(ctx->numType(), false, false)), nullptr),
      _externalName(std::move(externalName)) {

}

std::unique_ptr<ScalarExternalFunction> ScalarExternalFunction::create(MaximContext *ctx, llvm::Module *module,
                                                                       std::string externalName, std::string name,
                                                                       size_t paramCount) {
    return std::make_unique<ScalarExternalFunction>(ctx, module, externalName, name, paramCount);
}

std::unique_ptr<Value> ScalarExternalFunction::generate(ComposableModuleClassMethod *method,
                                                        const std::vector<std::unique_ptr<Value>> &params,
                                                        std::unique_ptr<VarArg> vararg) {
    auto floatTy = llvm::Type::getFloatTy(ctx()->llvm());
    auto extFunc = llvm::Function::Create(
        llvm::FunctionType::get(floatTy, std::vector<llvm::Type*>(params.size(), floatTy), false),
        llvm::Function::ExternalLinkage,
        _externalName, method->moduleClass()->module()
    );
    auto &b = method->builder();

    llvm::Value *isActive = ctx()->constInt(1, 0, false);

    std::vector<llvm::Value *> llVecs;
    llVecs.reserve(params.size());

    Num *firstParam = nullptr;
    for (const auto &param : params) {
        auto numParam = dynamic_cast<Num*>(param.get());
        assert(numParam);
        if (!firstParam) firstParam = numParam;
        llVecs.push_back(numParam->vec(b));
        isActive = b.CreateOr(isActive, numParam->active(b), "active");
    }
    assert(firstParam);

    auto numElements = ctx()->numType()->vecType()->getVectorNumElements();
    llvm::Value *res = llvm::UndefValue::get(ctx()->numType()->vecType());
    for (size_t i = 0; i < numElements; i++) {
        std::vector<llvm::Value *> llValues;
        llValues.reserve(llVecs.size());
        for (auto &llVec : llVecs) {
            llValues.push_back(b.CreateExtractElement(llVec, i, "vec.temp"));
        }

        auto singleResult = CreateCall(b, extFunc, llValues, "result.temp");
        res = b.CreateInsertElement(res, singleResult, i, "result");
    }

    SourcePos undefPos(-1, -1);
    return firstParam->withVec(b, res, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
}
