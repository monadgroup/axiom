#pragma once

#include <memory>

namespace MaximAst {
    class LValueExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitLValue(ComposableModuleClassMethod *method, Scope *scope, MaximAst::LValueExpression *expr);

}
