#include "CastVisitor.h"

#include "../../ast/CastExpression.h"
#include "../Num.h"
#include "../Node.h"
#include "../MaximContext.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitCast(Node *node, MaximAst::CastExpression *expr) {
    auto subexprVal = node->ctx()->assertNum(visitExpression(node, expr->expr.get()));

    if (expr->isConvert) {
        return node->ctx()->callConverter(expr->target->type, std::move(subexprVal), node, expr->startPos, expr->endPos);
    } else {
        return subexprVal->withForm(node->builder(), expr->target->type, expr->startPos, expr->endPos);
    }
}
