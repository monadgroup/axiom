#include "NoteVisitor.h"

#include "../../ast/NoteExpression.h"
#include "../Num.h"
#include "../ComposableModuleClass.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

std::unique_ptr<Value>
MaximCodegen::visitNote(ComposableModuleClassMethod *method, Scope *scope, MaximAst::NoteExpression *expr) {
    return Num::create(method->moduleClass()->ctx(), method->allocaBuilder(), expr->note, expr->note,
                       MaximCommon::FormType::NOTE, true, expr->startPos, expr->endPos);
}
