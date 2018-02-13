#include "MathVisitor.h"

#include "../../ast/MathExpression.h"
#include "../Node.h"
#include "../Value.h"
#include "../MaximContext.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitMath(Node *node, MaximAst::MathExpression *expr) {
    return node->ctx()->callOperator(
        expr->type,
        visitExpression(node, expr->left.get()),
        visitExpression(node, expr->right.get()),
        node, expr->startPos, expr->endPos
    );
}
