#pragma once

#include <memory>

namespace MaximAst {
    class NumberExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitNumber(ComposableModuleClassMethod *method, Scope *scope, MaximAst::NumberExpression *expr);

}
