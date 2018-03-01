#pragma once

#include <memory>

namespace MaximAst {
    class TupleExpression;
}

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class Scope;

    class Value;

    std::unique_ptr<Value> visitTuple(ComposableModuleClassMethod *method, Scope *scope, MaximAst::TupleExpression *expr);

}
