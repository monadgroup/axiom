#pragma once

#include <memory>

namespace MaximAst {
    class UnaryExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitUnary(Node *node, MaximAst::UnaryExpression *expr);

}
