#include "NumComparisonOperator.h"

#include "../MaximContext.h"
#include "../Num.h"

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

std::unique_ptr<Num> NumComparisonOperator::call(Builder &b, Num *numLeft, Num *numRight) {
    auto operatedInt = b.CreateFCmp(_op, numLeft->vec(b), numRight->vec(b), "op.ivec");
    auto isActive = b.CreateAnd(getActive(b, numLeft, numRight), operatedInt, "op.active");
    auto operatedVal = b.CreateUIToFP(operatedInt, context()->numType()->vecType(), "op.vec");

    auto undefPos = SourcePos(-1, -1);
    return numLeft->withVec(b, operatedVal, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
}
