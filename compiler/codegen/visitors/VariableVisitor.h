#pragma once

#include <memory>

namespace MaximAst {
    class VariableExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitVariable(Node *node, MaximAst::VariableExpression *expr);

}
