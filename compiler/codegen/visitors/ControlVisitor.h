#pragma once

#include <memory>

namespace MaximAst {
    class ControlExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value>
    visitControl(ComposableModuleClassMethod *method, Scope *scope, MaximAst::ControlExpression *expr);

}
