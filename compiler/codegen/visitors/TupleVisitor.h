#pragma once

#include <memory>

namespace MaximAst {
    class TupleExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitTuple(Node *node, MaximAst::TupleExpression *expr);

}
