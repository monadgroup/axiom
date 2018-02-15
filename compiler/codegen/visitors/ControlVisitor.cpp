#include "ControlVisitor.h"

#include "../Node.h"
#include "../Value.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitControl(Node *node, MaximAst::ControlExpression *expr) {
    return node->getControl(node->builder(), expr);
}
