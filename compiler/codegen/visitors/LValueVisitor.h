#pragma once

#include <memory>

namespace MaximAst {
    class LValueExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitLValue(Node *node, MaximAst::LValueExpression *expr);

}
