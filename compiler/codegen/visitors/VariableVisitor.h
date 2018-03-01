#pragma once

#include <memory>

namespace MaximAst {
    class VariableExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitVariable(ComposableModuleClassMethod *method, Scope *scope, MaximAst::VariableExpression *expr);

}
