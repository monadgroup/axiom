#pragma once

#include <memory>

namespace MaximAst {
    class PostfixExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value>
    visitPostfix(ComposableModuleClassMethod *method, Scope *scope, MaximAst::PostfixExpression *expr);

}
