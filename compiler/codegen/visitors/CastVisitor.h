#pragma once

#include <memory>

namespace MaximAst {
    class CastExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitCast(Node *node, MaximAst::CastExpression *expr);

}
