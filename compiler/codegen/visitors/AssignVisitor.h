#pragma once

#include <memory>

namespace MaximAst {
    class AssignExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value>
    visitAssign(ComposableModuleClassMethod *method, Scope *scope, MaximAst::AssignExpression *expr);

}
