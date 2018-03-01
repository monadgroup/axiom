#include "SawOscFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

SawOscFunction::SawOscFunction(MaximContext *ctx, llvm::Module *module) : PeriodicFunction(ctx, module, "sawOsc") {

}

std::unique_ptr<SawOscFunction> SawOscFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SawOscFunction>(ctx, module);
}

llvm::Value *SawOscFunction::nextValue(ComposableModuleClassMethod *method, llvm::Value *period) {
    auto &b = method->builder();
    auto normalPeriod = llvm::ConstantVector::getSplat(2, ctx()->constFloat(2));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    return b.CreateFSub(
        modInput,
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)),
        "result"
    );
}
