#pragma once

#include <memory>

namespace MaximAst {
    class MathExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitMath(ComposableModuleClassMethod *method, Scope *scope, MaximAst::MathExpression *expr);

}
