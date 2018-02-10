#pragma once

#include <memory>

namespace MaximAst {
    class Expression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitExpression(Node *node, MaximAst::Expression *expr);

}
