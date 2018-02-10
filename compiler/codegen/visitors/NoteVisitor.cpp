#include "NoteVisitor.h"

#include "../../ast/NoteExpression.h"
#include "../Num.h"
#include "../Node.h"

using namespace MaximCodegen;

std::unique_ptr<Value> MaximCodegen::visitNote(Node *node, MaximAst::NoteExpression *expr) {
    return Num::create(node->ctx(), expr->note, expr->note, MaximCommon::FormType::NOTE, true, expr->startPos,
                       expr->endPos);
}
