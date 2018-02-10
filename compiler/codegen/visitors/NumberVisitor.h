#pragma once

#include <memory>

namespace MaximAst {
    class NumberExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitNumber(Node *node, MaximAst::NumberExpression *expr);

}
