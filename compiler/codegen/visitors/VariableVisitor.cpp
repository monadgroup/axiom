#include "VariableVisitor.h"

#include "../../ast/VariableExpression.h"
#include "../Node.h"
#include "../MaximContext.h"
#include "../Value.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitVariable(Node *node, MaximAst::VariableExpression *expr) {
    auto val = node->getVariable(expr->name, expr->startPos, expr->endPos);
    if (!val) {
        throw MaximCommon::CompileError(
            "Ah hekkers mah dude! This variable hasn't been set yet!",
            expr->startPos, expr->endPos
        );
    }
    return val;
}
