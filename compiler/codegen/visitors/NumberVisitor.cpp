#include "NumberVisitor.h"

#include "../../ast/NumberExpression.h"
#include "../Num.h"
#include "../Node.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitNumber(Node *node, MaximAst::NumberExpression *expr) {
    return Num::create(node->ctx(), expr->value, expr->value, MaximCommon::FormType::LINEAR, true, expr->startPos,
                       expr->endPos);
}
