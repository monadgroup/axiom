#include "TupleVisitor.h"

#include <vector>

#include "../../ast/TupleExpression.h"
#include "../Tuple.h"
#include "../Node.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitTuple(Node *node, MaximAst::TupleExpression *expr) {
    Tuple::Storage values;
    values.reserve(expr->expressions.size());
    for (const auto &subExpr : expr->expressions) {
        values.push_back(visitExpression(node, subExpr.get()));
    }
    return Tuple::create(node->ctx(), values, node->builder(), expr->startPos, expr->endPos);
}
