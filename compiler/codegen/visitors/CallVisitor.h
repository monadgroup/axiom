#pragma once

#include <memory>

namespace MaximAst {
    class CallExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitCall(ComposableModuleClassMethod *method, Scope *scope, MaximAst::CallExpression *expr);

}
