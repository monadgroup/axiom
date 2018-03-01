#include "VariableVisitor.h"

#include "../../ast/VariableExpression.h"
#include "../Scope.h"
#include "../MaximContext.h"
#include "../Value.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitVariable(ComposableModuleClassMethod *method, Scope *scope, MaximAst::VariableExpression *expr) {
    auto val = scope->getVariable(expr->name, expr->startPos, expr->endPos);
    if (!val) {
        throw MaximCommon::CompileError(
            "Ah hekkers mah dude! This variable hasn't been set yet!",
            expr->startPos, expr->endPos
        );
    }
    return val;
}
