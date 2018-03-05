#include "LValueVisitor.h"

#include <vector>

#include "../../ast/LValueExpression.h"
#include "../Tuple.h"
#include "../ComposableModuleClassMethod.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value>
MaximCodegen::visitLValue(ComposableModuleClassMethod *method, Scope *scope, MaximAst::LValueExpression *expr) {
    Tuple::Storage values;
    values.reserve(expr->assignments.size());
    for (const auto &subExpr : expr->assignments) {
        values.push_back(visitExpression(method, scope, subExpr.get()));
    }
    return Tuple::create(method->moduleClass()->ctx(), std::move(values), method->builder(), expr->startPos,
                         expr->endPos);
}
