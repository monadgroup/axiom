#include "CallVisitor.h"

#include "../../ast/CallExpression.h"
#include "../ComposableModuleClassMethod.h"
#include "../Value.h"
#include "../MaximContext.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value>
MaximCodegen::visitCall(ComposableModuleClassMethod *method, Scope *scope, MaximAst::CallExpression *expr) {
    std::vector<std::unique_ptr<Value>> argVals;
    argVals.reserve(expr->arguments.size());
    for (const auto &subexpr : expr->arguments) {
        argVals.push_back(visitExpression(method, scope, subexpr.get()));
    }
    return method->moduleClass()->ctx()->callFunction(expr->name, std::move(argVals), method, expr->startPos,
                                                      expr->endPos);
}
