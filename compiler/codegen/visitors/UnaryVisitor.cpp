#include "UnaryVisitor.h"

#include "../../ast/UnaryExpression.h"
#include "../Node.h"
#include "../MaximContext.h"
#include "../Num.h"
#include "../Tuple.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

static std::unique_ptr<Num> visitSingleUnary(Node *node, MaximAst::UnaryExpression *expr, std::unique_ptr<Value> val) {
    auto num = node->ctx()->assertNum(std::move(val));

    switch (expr->type) {
        case MaximAst::UnaryExpression::Type::POSITIVE: return std::move(num);
        case MaximAst::UnaryExpression::Type::NEGATIVE:
        {
            auto numVec = num->vec(node->builder());
            auto negVec = node->builder().CreateFNeg(numVec, "negative");
            return num->withVec(node->builder(), negVec, expr->startPos, expr->endPos);
        }
        case MaximAst::UnaryExpression::Type::NOT:
        {
            auto numVec = num->vec(node->builder());
            auto zeroFloat = node->ctx()->constFloat(0);
            auto zeroVec = llvm::ConstantVector::get({zeroFloat, zeroFloat});
            auto compareResult = node->builder().CreateFCmp(llvm::CmpInst::Predicate::FCMP_OEQ, numVec, zeroVec, "equalzero");
            auto compareFloat = node->builder().CreateUIToFP(compareResult, node->ctx()->numType()->vecType());
            return num->withVec(node->builder(), compareFloat, expr->startPos, expr->endPos);
        }
    }

    assert(false); throw;
}

std::unique_ptr<Value> MaximCodegen::visitUnary(Node *node, MaximAst::UnaryExpression *expr) {
    auto exprVal = visitExpression(node, expr->expr.get());

    if (auto tuple = dynamic_cast<Tuple*>(exprVal.get())) {
        Tuple::Storage tupleVals;
        tupleVals.reserve(tuple->type()->types().size());
        for (size_t i = 0; i < tuple->type()->types().size(); i++) {
            tupleVals.push_back(visitSingleUnary(node, expr, tuple->atIndex(i, node->builder(), expr->startPos, expr->endPos)));
        }
        return Tuple::create(node->ctx(), std::move(tupleVals), node->builder(), expr->startPos, expr->endPos);
    }

    return visitSingleUnary(node, expr, std::move(exprVal));
}
