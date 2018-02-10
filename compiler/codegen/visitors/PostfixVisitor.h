#pragma once

#include <memory>

namespace MaximAst {
    class PostfixExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitPostfix(Node *node, MaximAst::PostfixExpression *expr);

}
