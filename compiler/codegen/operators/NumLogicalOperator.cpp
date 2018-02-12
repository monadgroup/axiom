#include "NumLogicalOperator.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Constants.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

NumLogicalOperator::NumLogicalOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                                       llvm::Instruction::BinaryOps op)
    : NumOperator(context, type, activeMode), _op(op) {
}

std::unique_ptr<NumLogicalOperator> NumLogicalOperator::create(MaximContext *context, MaximCommon::OperatorType type,
                                                               ActiveMode activeMode, llvm::Instruction::BinaryOps op) {
    return std::make_unique<NumLogicalOperator>(context, type, activeMode, op);
}

std::unique_ptr<Num> NumLogicalOperator::call(Builder &b, Num *numLeft, Num *numRight) {
    auto zeroConst = context()->constFloat(0);
    auto zeroVec = llvm::ConstantVector::get({zeroConst, zeroConst});

    auto operatedInt = b.CreateBinOp(
        _op,
        b.CreateFCmp(llvm::CmpInst::Predicate::FCMP_ONE, numLeft->vec(b), zeroVec, "logical.left"),
        b.CreateFCmp(llvm::CmpInst::Predicate::FCMP_ONE, numRight->vec(b), zeroVec, "logical.right"),
        "op.ivec"
    );
    auto isActive = b.CreateAnd(getActive(b, numLeft, numRight), operatedInt, "op.active");
    auto operatedFloat = b.CreateUIToFP(operatedInt, context()->numType()->vecType(), "op.vec");

    auto undefPos = SourcePos(-1, -1);
    return numLeft->withVec(b, operatedFloat, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
}
