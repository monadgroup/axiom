#pragma once

#include <memory>

namespace MaximAst {
    class CastExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitCast(ComposableModuleClassMethod *method, Scope *scope, MaximAst::CastExpression *expr);

}
