#pragma once

#include <memory>

namespace MaximAst {
    class MathExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitMath(Node *node, MaximAst::MathExpression *expr);

}
