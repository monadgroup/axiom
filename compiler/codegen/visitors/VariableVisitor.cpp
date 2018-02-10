#include "VariableVisitor.h"

#include "../../ast/VariableExpression.h"
#include "../Node.h"
#include "../MaximContext.h"
#include "../CodegenError.h"
#include "../Value.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitVariable(Node *node, MaximAst::VariableExpression *expr) {
    auto val = node->getVariable(expr->name);
    if (!val) {
        throw CodegenError(
            "Ah hekkers mah dude! This variable hasn't been set yet!",
            expr->startPos, expr->endPos
        );
    }
    return val->withSource(expr->startPos, expr->endPos);
}
