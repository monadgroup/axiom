#include "CallVisitor.h"

#include "../../ast/CallExpression.h"
#include "../Node.h"
#include "../MaximContext.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitCall(Node *node, MaximAst::CallExpression *expr) {
    std::vector<std::unique_ptr<Value>> argVals;
    argVals.reserve(expr->arguments.size());
    for (const auto &subexpr : expr->arguments) {
        argVals.push_back(visitExpression(node, subexpr.get()));
    }
    return node->ctx()->callFunction(expr->name, argVals);
}
