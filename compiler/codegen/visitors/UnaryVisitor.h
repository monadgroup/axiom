#pragma once

#include <memory>

namespace MaximAst {
    class UnaryExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitUnary(ComposableModuleClassMethod *method, Scope *scope, MaximAst::UnaryExpression *expr);

}
