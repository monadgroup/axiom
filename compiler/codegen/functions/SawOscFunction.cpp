#include "SawOscFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

SawOscFunction::SawOscFunction(MaximContext *context) : PeriodicFunction(context, "sawOsc") {

}

std::unique_ptr<SawOscFunction> SawOscFunction::create(MaximContext *context) {
    return std::make_unique<SawOscFunction>(context);
}

llvm::Value *SawOscFunction::nextValue(llvm::Value *period, Builder &b, llvm::Module *module) {
    auto normalPeriod = llvm::ConstantVector::getSplat(2, context()->constFloat(2));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    return b.CreateFSub(
        modInput,
        llvm::ConstantVector::getSplat(2, context()->constFloat(1)),
        "result"
    );
}
