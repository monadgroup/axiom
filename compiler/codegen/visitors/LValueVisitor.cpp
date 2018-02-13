#include "LValueVisitor.h"

#include <vector>

#include "../../ast/LValueExpression.h"
#include "../Tuple.h"
#include "../Node.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitLValue(Node *node, MaximAst::LValueExpression *expr) {
    Tuple::Storage values;
    values.reserve(expr->assignments.size());
    for (const auto &subExpr : expr->assignments) {
        values.push_back(visitExpression(node, subExpr.get()));
    }
    return Tuple::create(node->ctx(), std::move(values), node->builder(), expr->startPos, expr->endPos);
}
