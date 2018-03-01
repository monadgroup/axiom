#include "TupleVisitor.h"

#include <vector>

#include "../../ast/TupleExpression.h"
#include "../Tuple.h"
#include "../ComposableModuleClass.h"
#include "../ComposableModuleClassMethod.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitTuple(ComposableModuleClassMethod *method, Scope *scope, MaximAst::TupleExpression *expr) {
    Tuple::Storage values;
    values.reserve(expr->expressions.size());
    for (const auto &subExpr : expr->expressions) {
        values.push_back(visitExpression(method, scope, subExpr.get()));
    }
    return Tuple::create(method->moduleClass()->ctx(), std::move(values), method->builder(), expr->startPos, expr->endPos);
}
