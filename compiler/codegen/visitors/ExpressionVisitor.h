#pragma once

#include <memory>

namespace MaximAst {
    class Expression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value>
    visitExpression(ComposableModuleClassMethod *method, Scope *scope, MaximAst::Expression *expr);

}
