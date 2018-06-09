#include "NumComparisonOperator.h"

#include "../MaximContext.h"
#include "../Num.h"
#include "../ModuleClassMethod.h"

using namespace MaximCodegen;

NumComparisonOperator::NumComparisonOperator(MaximContext *context, MaximCommon::OperatorType type,
                                             ActiveMode activeMode, llvm::CmpInst::Predicate op)
    : NumOperator(context, type, activeMode), _op(op) {
}

std::unique_ptr<NumComparisonOperator> NumComparisonOperator::create(MaximContext *context,
                                                                     MaximCommon::OperatorType type,
                                                                     ActiveMode activeMode,
                                                                     llvm::CmpInst::Predicate op) {
    return std::make_unique<NumComparisonOperator>(context, type, activeMode, op);
}

std::unique_ptr<Num> NumComparisonOperator::call(ModuleClassMethod *method, Num *numLeft, Num *numRight) {
    auto &b = method->builder();
    auto operatedInt = b.CreateFCmp(_op, numLeft->vec(b), numRight->vec(b), "op.ivec");
    auto isActive = b.CreateAnd(getActive(b, numLeft, numRight), b.CreateOr(b.CreateExtractElement(operatedInt, (uint64_t) 0), b.CreateExtractElement(operatedInt, (uint64_t) 1)), "op.active");
    auto operatedVal = b.CreateUIToFP(operatedInt, context()->numType()->vecType(), "op.vec");

    auto undefPos = SourcePos(-1, -1);
    auto newNum = Num::create(context(), method->allocaBuilder(), undefPos, undefPos);
    newNum->setVec(b, operatedVal);
    newNum->setForm(b, numLeft->form(b));
    newNum->setActive(b, isActive);
    return std::move(newNum);
}
