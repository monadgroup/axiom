#include "SinOscFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

SinOscFunction::SinOscFunction(MaximContext *ctx, llvm::Module *module) : PeriodicFunction(ctx, module, "sinOsc") {

}

std::unique_ptr<SinOscFunction> SinOscFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SinOscFunction>(ctx, module);
}

llvm::Value *SinOscFunction::nextValue(ComposableModuleClassMethod *method, llvm::Value *period) {
    auto sinFunc = llvm::Intrinsic::getDeclaration(
        method->moduleClass()->module(),
        llvm::Intrinsic::ID::sin, {ctx()->numType()->vecType()}
    );

    auto &b = method->builder();

    auto sinPeriod = b.CreateFMul(
        period,
        ctx()->constFloatVec(M_PI * 2),
        "sinperiod"
    );
    return CreateCall(b, sinFunc, {sinPeriod}, "result");
}
