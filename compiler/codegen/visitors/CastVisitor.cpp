#include "CastVisitor.h"

#include "../../ast/CastExpression.h"
#include "../Num.h"
#include "../ComposableModuleClassMethod.h"
#include "../MaximContext.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

std::unique_ptr<Value>
MaximCodegen::visitCast(ComposableModuleClassMethod *method, Scope *scope, MaximAst::CastExpression *expr) {
    auto subexprVal = method->moduleClass()->ctx()->assertNum(visitExpression(method, scope, expr->expr.get()));

    if (expr->isConvert) {
        return method->moduleClass()->ctx()->callConverter(expr->target->type, std::move(subexprVal), method,
                                                           expr->startPos, expr->endPos);
    } else {
        auto newNum = Num::create(method->moduleClass()->ctx(), method->allocaBuilder(), expr->startPos, expr->endPos);
        newNum->setVec(method->builder(), subexprVal->vec(method->builder()));
        newNum->setForm(method->builder(), expr->target->type);
        newNum->setActive(method->builder(), subexprVal->active(method->builder()));
        return std::move(newNum);
    }
}
