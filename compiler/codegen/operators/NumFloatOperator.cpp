#include "NumFloatOperator.h"

#include "../MaximContext.h"
#include "../Num.h"
#include "../Node.h"

using namespace MaximCodegen;

NumFloatOperator::NumFloatOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                                         llvm::Instruction::BinaryOps op)
    : NumOperator(context, type, activeMode), _op(op) {
}

std::unique_ptr<NumFloatOperator> NumFloatOperator::create(MaximContext *context, MaximCommon::OperatorType type,
                                                           ActiveMode activeMode, llvm::Instruction::BinaryOps op) {
    return std::make_unique<NumFloatOperator>(context, type, activeMode, op);
}

std::unique_ptr<Num> NumFloatOperator::call(Node *node, Num *numLeft, Num *numRight) {
    auto &b = node->builder();
    auto operatedVal = b.CreateBinOp(_op, numLeft->vec(b), numRight->vec(b), "op.vec");
    auto isActive = getActive(b, numLeft, numRight);

    auto undefPos = SourcePos(-1, -1);
    return numLeft->withVec(b, operatedVal, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
}
