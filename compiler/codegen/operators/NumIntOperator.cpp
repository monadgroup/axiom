#include "NumIntOperator.h"

#include "../MaximContext.h"
#include "../Num.h"
#include "../Node.h"

using namespace MaximCodegen;

NumIntOperator::NumIntOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode,
                               llvm::Instruction::BinaryOps op, bool isSigned)
    : NumOperator(context, type, activeMode), _op(op), _isSigned(isSigned) {
}

std::unique_ptr<NumIntOperator> NumIntOperator::create(MaximContext *context, MaximCommon::OperatorType type,
                                                       ActiveMode activeMode, llvm::Instruction::BinaryOps op,
                                                       bool isSigned) {
    return std::make_unique<NumIntOperator>(context, type, activeMode, op, isSigned);
}

std::unique_ptr<Num> NumIntOperator::call(Node *node, Num *numLeft, Num *numRight) {
    auto &b = node->builder();
    auto floatVec = context()->numType()->vecType();
    auto intVec = llvm::VectorType::get(llvm::Type::getInt32Ty(context()->llvm()), floatVec->getVectorNumElements());

    auto operatedInt = b.CreateBinOp(
        _op,
        _isSigned ? b.CreateFPToSI(numLeft->vec(b), intVec, "left.int") : b.CreateFPToUI(numLeft->vec(b), intVec,
                                                                                         "left.int"),
        _isSigned ? b.CreateFPToSI(numRight->vec(b), intVec, "right.int") : b.CreateFPToUI(numRight->vec(b), intVec,
                                                                                           "right.int"),
        "op.ivec"
    );
    auto isActive = getActive(b, numLeft, numRight);
    auto operatedFloat = _isSigned ? b.CreateSIToFP(operatedInt, floatVec, "op.vec") : b.CreateUIToFP(operatedInt,
                                                                                                      floatVec,
                                                                                                      "op.vec");

    auto undefPos = SourcePos(-1, -1);
    return numLeft->withVec(b, operatedFloat, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
}
