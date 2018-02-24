#include "TriOscFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

TriOscFunction::TriOscFunction(MaximContext *context) : PeriodicFunction(context, "triOsc") {

}

std::unique_ptr<TriOscFunction> TriOscFunction::create(MaximContext *context) {
    return std::make_unique<TriOscFunction>(context);
}

llvm::Value *TriOscFunction::nextValue(llvm::Value *period, Builder &b, llvm::Module *module) {
    auto absFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::fabs,
                                                   {context()->numType()->vecType()});

    auto normalPeriod = llvm::ConstantVector::getSplat(2, context()->constFloat(4));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    auto subInput = b.CreateFSub(
        modInput,
        llvm::ConstantVector::getSplat(2, context()->constFloat(2)),
        "inputsub"
    );
    auto normalized = CreateCall(b, absFunc, {subInput}, "normalized");
    return b.CreateFSub(
        normalized,
        llvm::ConstantVector::getSplat(2, context()->constFloat(1)),
        "result"
    );
}
