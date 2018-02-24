#include "SinOscFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

SinOscFunction::SinOscFunction(MaximContext *context) : PeriodicFunction(context, "sinOsc") {

}

std::unique_ptr<SinOscFunction> SinOscFunction::create(MaximContext *context) {
    return std::make_unique<SinOscFunction>(context);
}

llvm::Value *SinOscFunction::nextValue(llvm::Value *period, Builder &b, llvm::Module *module) {
    auto sinFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::sin, {context()->numType()->vecType()});

    auto sinPeriod = b.CreateFMul(
        period,
        llvm::ConstantVector::getSplat(2, context()->constFloat(M_PI * 2)),
        "sinperiod"
    );
    return CreateCall(b, sinFunc, {sinPeriod}, "result");
}
