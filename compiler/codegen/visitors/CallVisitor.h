#pragma once

#include <memory>

namespace MaximAst {
    class CallExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitCall(Node *node, MaximAst::CallExpression *expr);

}
