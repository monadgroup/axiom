#include "NumIntrinsicOperator.h"

#include "../MaximContext.h"
#include "../Num.h"
#include "../ModuleClassMethod.h"
#include "../ModuleClass.h"

using namespace MaximCodegen;

NumIntrinsicOperator::NumIntrinsicOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                                           llvm::Intrinsic::ID id)
    : NumOperator(context, type, activeMode), _id(id) {

}

std::unique_ptr<NumIntrinsicOperator>
NumIntrinsicOperator::create(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                             llvm::Intrinsic::ID id) {
    return std::make_unique<NumIntrinsicOperator>(context, type, activeMode, id);
}

std::unique_ptr<Num> NumIntrinsicOperator::call(ModuleClassMethod *method, Num *numLeft, Num *numRight) {
    auto &b = method->builder();
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), _id,
                                                        context()->numType()->vecType());
    auto operatedVal = CreateCall(b, powIntrinsic, {
        numLeft->vec(b),
        numRight->vec(b)
    }, "op.vec");
    auto isActive = getActive(b, numLeft, numRight);

    auto undefPos = SourcePos(-1, -1);
    auto newNum = Num::create(context(), method->allocaBuilder(), undefPos, undefPos);
    newNum->setVec(b, operatedVal);
    newNum->setForm(b, numLeft->form(b));
    newNum->setActive(b, isActive);
    return newNum;
}
