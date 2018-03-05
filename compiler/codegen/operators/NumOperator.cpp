#include "NumOperator.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

NumOperator::NumOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode)
    : Operator(context, type, context->numType(), context->numType()), _activeMode(activeMode) {
}

std::unique_ptr<Value>
NumOperator::call(ModuleClassMethod *method, std::unique_ptr<Value> left, std::unique_ptr<Value> right,
                  SourcePos startPos,
                  SourcePos endPos) {
    auto leftNum = dynamic_cast<Num *>(left.get());
    auto rightNum = dynamic_cast<Num *>(right.get());
    assert(leftNum && rightNum);

    return call(method, leftNum, rightNum)->withSource(startPos, endPos);
}

llvm::Value *NumOperator::getActive(Builder &b, Num *left, Num *right) {
    switch (_activeMode) {
        case ActiveMode::ANY_INPUT:
            return b.CreateOr(left->active(b), right->active(b), "op.active");
        case ActiveMode::ALL_INPUTS:
            return b.CreateAnd(left->active(b), right->active(b), "op.active");
        case ActiveMode::FIRST_INPUT:
            return left->active(b);
    }

    assert(false);
    throw;
}
