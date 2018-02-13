#include "NumIntrinsicOperator.h"

#include "../MaximContext.h"
#include "../Num.h"
#include "../Node.h"

using namespace MaximCodegen;

NumIntrinsicOperator::NumIntrinsicOperator(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::Intrinsic::ID id)
    : NumOperator(context, type, activeMode), _id(id) {

}

std::unique_ptr<NumIntrinsicOperator> NumIntrinsicOperator::create(MaximContext *context, MaximCommon::OperatorType type, ActiveMode activeMode, llvm::Intrinsic::ID id) {
    return std::make_unique<NumIntrinsicOperator>(context, type, activeMode, id);
}

std::unique_ptr<Num> NumIntrinsicOperator::call(Node *node, Num *numLeft, Num *numRight) {
    auto &b = node->builder();
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(node->module(), _id, context()->numType()->vecType());
    auto operatedVal = CreateCall(b, powIntrinsic, {
        numLeft->vec(b),
        numRight->vec(b)
    }, "op.vec");
    auto isActive = getActive(b, numLeft, numRight);

    auto undefPos = SourcePos(-1, -1);
    return numLeft->withVec(b, operatedVal, undefPos, undefPos)->withActive(b, isActive, undefPos, undefPos);
}
