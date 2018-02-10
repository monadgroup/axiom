#pragma once

#include <memory>

namespace MaximAst {
    class ControlExpression;
}

namespace MaximCodegen {

    class Node;

    class Value;

    std::unique_ptr<Value> visitControl(Node *node, MaximAst::ControlExpression *expr);

}
