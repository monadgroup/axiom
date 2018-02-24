#include "SqrOscFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

SqrOscFunction::SqrOscFunction(MaximContext *context) : PeriodicFunction(context, "sqrOsc") {

}

std::unique_ptr<SqrOscFunction> SqrOscFunction::create(MaximContext *context) {
    return std::make_unique<SqrOscFunction>(context);
}

llvm::Value *SqrOscFunction::nextValue(llvm::Value *period, Builder &b, llvm::Module *module) {
    auto floorFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::floor,
                                                     {context()->numType()->vecType()});

    auto normalPeriod = llvm::ConstantVector::getSplat(2, context()->constFloat(2));
    auto inputVal = b.CreateFMul(period, normalPeriod, "inputval");
    auto modInput = b.CreateFRem(inputVal, normalPeriod, "inputmod");
    auto floorInput = CreateCall(b, floorFunc, {modInput}, "inputfloor");
    auto normalized = b.CreateFMul(
        floorInput,
        llvm::ConstantVector::getSplat(2, context()->constFloat(2)),
        "normalized"
    );
    return b.CreateFSub(
        normalized,
        llvm::ConstantVector::getSplat(2, context()->constFloat(1)),
        "result"
    );
}
