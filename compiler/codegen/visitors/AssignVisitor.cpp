#include "AssignVisitor.h"

#include "../../ast/AssignExpression.h"
#include "../ComposableModuleClassMethod.h"
#include "../Scope.h"
#include "../MaximContext.h"
#include "../Operator.h"
#include "../Tuple.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value>
MaximCodegen::visitAssign(ComposableModuleClassMethod *method, Scope *scope, MaximAst::AssignExpression *expr) {
    std::unique_ptr<Value> rightVal;
    if (expr->type == MaximCommon::OperatorType::IDENTITY) rightVal = visitExpression(method, scope, expr->right.get());
    else {
        rightVal = method->moduleClass()->ctx()->callOperator(
            expr->type,
            visitExpression(method, scope, expr->left.get()),
            visitExpression(method, scope, expr->right.get()),
            method, expr->startPos, expr->endPos
        );
    }

    auto leftTuple = expr->left->assignments.size() != 1;
    auto rightTuple = dynamic_cast<Tuple *>(rightVal.get());

    if (leftTuple && rightTuple) {
        auto leftSize = expr->left->assignments.size();
        auto rightSize = rightTuple->type()->types().size();
        if (leftSize != rightSize) {
            throw MaximCommon::CompileError(
                "OOOOOOOOOOOOOOOOOOOOOOYYYYYY!!!!1! You're trying to assign " + std::to_string(rightSize) +
                " values to " + std::to_string(leftSize) + " ones!",
                expr->startPos, expr->endPos
            );
        }

        for (size_t i = 0; i < leftSize; i++) {
            auto assignment = expr->left->assignments[i].get();
            scope->setAssignable(
                method, assignment,
                rightTuple->atIndex(i, method->builder(), assignment->startPos, assignment->endPos)->clone()
            );
        }

        return rightVal;
    }

    for (const auto &assignment : expr->left->assignments) {
        scope->setAssignable(method, assignment.get(), rightVal->clone());
    }

    if (leftTuple) {
        Tuple::Storage rightVals;
        rightVals.reserve(expr->left->assignments.size());
        for (size_t i = 0; i < expr->left->assignments.size(); i++) {
            rightVals.push_back(rightVal->clone());
        }
        return Tuple::create(method->moduleClass()->ctx(), std::move(rightVals), method->builder(),
                             method->allocaBuilder(),
                             expr->startPos, expr->endPos);
    }

    return rightVal;
}
