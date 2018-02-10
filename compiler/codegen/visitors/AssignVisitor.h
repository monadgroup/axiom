#pragma once

#include <memory>

namespace MaximAst {
    class AssignExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitAssign(Node *node, MaximAst::AssignExpression *expr);

}
