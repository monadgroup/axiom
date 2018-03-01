#include "SqrOscFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

SqrOscFunction::SqrOscFunction(MaximContext *ctx, llvm::Module *module) : PeriodicFunction(ctx, module, "sqrOsc") {

}

std::unique_ptr<SqrOscFunction> SqrOscFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SqrOscFunction>(ctx, module);
}

llvm::Value *SqrOscFunction::nextValue(ComposableModuleClassMethod *method, llvm::Value *period) {
    auto floorFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::floor,
                                                     {ctx()->numType()->vecType()});

    auto &b = method->builder();

    auto normalPeriod = llvm::ConstantVector::getSplat(2, ctx()->constFloat(2));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    auto floorInput = CreateCall(b, floorFunc, {modInput}, "inputfloor");
    auto normalized = b.CreateFMul(
        floorInput,
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(2)),
        "normalized"
    );
    return b.CreateFSub(
        normalized,
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)),
        "result"
    );
}
