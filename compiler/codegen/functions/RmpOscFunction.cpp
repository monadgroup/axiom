#include "RmpOscFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

RmpOscFunction::RmpOscFunction(MaximContext *ctx, llvm::Module *module) : PeriodicFunction(ctx, module, "rmpOsc") {

}

std::unique_ptr<RmpOscFunction> RmpOscFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<RmpOscFunction>(ctx, module);
}

llvm::Value *RmpOscFunction::nextValue(ComposableModuleClassMethod *method, llvm::Value *period) {
    auto &b = method->builder();
    auto normalPeriod = llvm::ConstantVector::getSplat(2, ctx()->constFloat(2));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    return b.CreateFSub(
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)),
        modInput,
        "result"
    );
}
