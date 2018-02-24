#include "RmpOscFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

RmpOscFunction::RmpOscFunction(MaximContext *context) : PeriodicFunction(context, "rmpOsc") {

}

std::unique_ptr<RmpOscFunction> RmpOscFunction::create(MaximContext *context) {
    return std::make_unique<RmpOscFunction>(context);
}

llvm::Value* RmpOscFunction::nextValue(llvm::Value *period, Builder &b, llvm::Module *module) {
    auto normalPeriod = llvm::ConstantVector::getSplat(2, context()->constFloat(2));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    return b.CreateFSub(
        llvm::ConstantVector::getSplat(2, context()->constFloat(1)),
        modInput,
        "result"
    );
}
