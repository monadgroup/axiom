#include "MathVisitor.h"

#include "../../ast/MathExpression.h"
#include "../ComposableModuleClassMethod.h"
#include "../Value.h"
#include "../MaximContext.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value>
MaximCodegen::visitMath(ComposableModuleClassMethod *method, Scope *scope, MaximAst::MathExpression *expr) {
    return method->moduleClass()->ctx()->callOperator(
        expr->type,
        visitExpression(method, scope, expr->left.get()),
        visitExpression(method, scope, expr->right.get()),
        method, expr->startPos, expr->endPos
    );
}
