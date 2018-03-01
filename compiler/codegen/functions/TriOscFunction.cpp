#include "TriOscFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

TriOscFunction::TriOscFunction(MaximContext *ctx, llvm::Module *module) : PeriodicFunction(ctx, module, "triOsc") {

}

std::unique_ptr<TriOscFunction> TriOscFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<TriOscFunction>(ctx, module);
}

llvm::Value *TriOscFunction::nextValue(ComposableModuleClassMethod *method, llvm::Value *period) {
    auto absFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::fabs,
                                                   {ctx()->numType()->vecType()});

    auto &b = method->builder();

    auto normalPeriod = llvm::ConstantVector::getSplat(2, ctx()->constFloat(4));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    auto subInput = b.CreateFSub(
        modInput,
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(2)),
        "inputsub"
    );
    auto normalized = CreateCall(b, absFunc, {subInput}, "normalized");
    return b.CreateFSub(
        normalized,
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)),
        "result"
    );
}
