#include "AssignVisitor.h"

#include "../../ast/AssignExpression.h"
#include "../Node.h"
#include "../MaximContext.h"
#include "../Operator.h"
#include "../Tuple.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitAssign(Node *node, MaximAst::AssignExpression *expr) {
    std::unique_ptr<Value> rightVal;
    if (expr->type == MaximCommon::OperatorType::IDENTITY) rightVal = visitExpression(node, expr->right.get());
    else {
        rightVal = node->ctx()->callOperator(
            expr->type,
            visitExpression(node, expr->left.get()),
            visitExpression(node, expr->right.get()),
            node->builder(), expr->startPos, expr->endPos
        );
    }

    auto leftTuple = expr->left->assignments.size() > 1;
    auto rightTuple = dynamic_cast<Tuple *>(rightVal.get());

    if (leftTuple && rightTuple) {
        auto leftSize = expr->left->assignments.size();
        auto rightSize = rightTuple->type()->types().size();
        if (leftSize != rightSize) {
            throw CodegenError(
                "OOOOOOOOOOOOOOOOOOOOOOYYYYYY!!!!1! You're trying to assign " + std::to_string(rightSize) +
                " values to " + std::to_string(leftSize) + " ones!",
                expr->startPos, expr->endPos
            );
        }

        for (size_t i = 0; i < leftSize; i++) {
            auto assignment = expr->left->assignments[i].get();
            node->setAssignable(
                assignment,
                rightTuple->atIndex(i, node->builder(), assignment->startPos, assignment->endPos)->clone()
            );
        }
    } else {
        for (const auto &assignment : expr->left->assignments) {
            node->setAssignable(
                assignment.get(),
                rightVal->clone()
            );
        }
    }

    return rightVal;
}
