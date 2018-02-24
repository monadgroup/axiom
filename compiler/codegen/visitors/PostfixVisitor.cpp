#include "PostfixVisitor.h"

#include "../../ast/PostfixExpression.h"
#include "../Node.h"
#include "../MaximContext.h"
#include "../Num.h"
#include "../Tuple.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

static std::unique_ptr<Num>
visitSinglePostfix(Node *node, MaximAst::PostfixExpression *expr, MaximAst::AssignableExpression *assignable) {
    auto num = node->ctx()->assertNum(visitExpression(node, assignable));
    auto numVec = num->vec(node->builder());

    auto constOne = node->ctx()->constFloat(1);
    auto constOneVec = llvm::ConstantVector::get({constOne, constOne});

    switch (expr->type) {
        case MaximAst::PostfixExpression::Type::INCREMENT:
            numVec = node->builder().CreateBinOp(llvm::Instruction::BinaryOps::FAdd, numVec, constOneVec,
                                                 "incremented");
            break;
        case MaximAst::PostfixExpression::Type::DECREMENT:
            numVec = node->builder().CreateBinOp(llvm::Instruction::BinaryOps::FSub, numVec, constOneVec,
                                                 "decremented");
            break;
    }

    auto newNum = num->withVec(node->builder(), numVec, expr->startPos, expr->endPos);
    node->setAssignable(node->builder(), assignable, std::move(newNum));

    return num;
}

std::unique_ptr<Value> MaximCodegen::visitPostfix(Node *node, MaximAst::PostfixExpression *expr) {
    std::vector<std::unique_ptr<Value>> tupleVals;
    tupleVals.reserve(expr->left->assignments.size());

    for (const auto &assignment : expr->left->assignments) {
        tupleVals.push_back(visitSinglePostfix(node, expr, assignment.get()));
    }

    if (tupleVals.size() == 1) return std::move(tupleVals[0]);
    else return Tuple::create(node->ctx(), std::move(tupleVals), node->builder(), expr->startPos, expr->endPos);
}
