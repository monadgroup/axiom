#include "ControlVisitor.h"

#include "../Scope.h"
#include "../ComposableModuleClassMethod.h"
#include "../Value.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitControl(ComposableModuleClassMethod *method, Scope *scope, MaximAst::ControlExpression *expr) {
    return scope->getControl(method, expr);
}
