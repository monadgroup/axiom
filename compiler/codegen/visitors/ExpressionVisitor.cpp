#include "ExpressionVisitor.h"

#include <cassert>

#include "../../ast/AssignExpression.h"
#include "../../ast/CallExpression.h"
#include "../../ast/CastExpression.h"
#include "../../ast/ControlExpression.h"
#include "../../ast/MathExpression.h"
#include "../../ast/NoteExpression.h"
#include "../../ast/NumberExpression.h"
#include "../../ast/PostfixExpression.h"
#include "../../ast/TupleExpression.h"
#include "../../ast/UnaryExpression.h"
#include "../../ast/VariableExpression.h"

#include "AssignVisitor.h"
#include "CallVisitor.h"
#include "CastVisitor.h"
#include "ControlVisitor.h"
#include "LValueVisitor.h"
#include "MathVisitor.h"
#include "NoteVisitor.h"
#include "NumberVisitor.h"
#include "PostfixVisitor.h"
#include "TupleVisitor.h"
#include "UnaryVisitor.h"
#include "VariableVisitor.h"

#include "../Value.h"

using namespace MaximCodegen;
using namespace MaximAst;

std::unique_ptr<Value>
MaximCodegen::visitExpression(ComposableModuleClassMethod *method, Scope *scope, MaximAst::Expression *expr) {
    if (auto assign = dynamic_cast<AssignExpression *>(expr)) return visitAssign(method, scope, assign);
    if (auto call = dynamic_cast<CallExpression *>(expr)) return visitCall(method, scope, call);
    if (auto cast = dynamic_cast<CastExpression *>(expr)) return visitCast(method, scope, cast);
    if (auto control = dynamic_cast<ControlExpression *>(expr)) return visitControl(method, scope, control);
    if (auto lvalue = dynamic_cast<LValueExpression *>(expr)) return visitLValue(method, scope, lvalue);
    if (auto math = dynamic_cast<MathExpression *>(expr)) return visitMath(method, scope, math);
    if (auto note = dynamic_cast<NoteExpression *>(expr)) return visitNote(method, scope, note);
    if (auto number = dynamic_cast<NumberExpression *>(expr)) return visitNumber(method, scope, number);
    if (auto postfix = dynamic_cast<PostfixExpression *>(expr)) return visitPostfix(method, scope, postfix);
    if (auto tuple = dynamic_cast<TupleExpression *>(expr)) return visitTuple(method, scope, tuple);
    if (auto unary = dynamic_cast<UnaryExpression *>(expr)) return visitUnary(method, scope, unary);
    if (auto variable = dynamic_cast<VariableExpression *>(expr)) return visitVariable(method, scope, variable);

    assert(false);
    throw;
}
