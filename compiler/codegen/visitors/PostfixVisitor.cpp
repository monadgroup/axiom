#include "PostfixVisitor.h"

#include "../../ast/PostfixExpression.h"
#include "../ComposableModuleClassMethod.h"
#include "../Scope.h"
#include "../MaximContext.h"
#include "../Num.h"
#include "../Tuple.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

static std::unique_ptr<Num>
visitSinglePostfix(ComposableModuleClassMethod *method, Scope *scope, MaximAst::PostfixExpression *expr,
                   MaximAst::AssignableExpression *assignable) {
    auto num = method->moduleClass()->ctx()->assertNum(visitExpression(method, scope, assignable));
    auto numVec = num->vec(method->builder());

    auto constOneVec = method->moduleClass()->ctx()->constFloatVec(1);

    switch (expr->type) {
        case MaximAst::PostfixExpression::Type::INCREMENT:
            numVec = method->builder().CreateBinOp(llvm::Instruction::BinaryOps::FAdd, numVec, constOneVec,
                                                   "incremented");
            break;
        case MaximAst::PostfixExpression::Type::DECREMENT:
            numVec = method->builder().CreateBinOp(llvm::Instruction::BinaryOps::FSub, numVec, constOneVec,
                                                   "decremented");
            break;
    }

    auto newNum = Num::create(method->moduleClass()->ctx(), method->allocaBuilder(), expr->startPos, expr->endPos);
    newNum->setVec(method->builder(), numVec);
    newNum->setForm(method->builder(), num->form(method->builder()));
    newNum->setActive(method->builder(), num->active(method->builder()));
    scope->setAssignable(method, assignable, std::move(newNum));

    return num;
}

std::unique_ptr<Value>
MaximCodegen::visitPostfix(ComposableModuleClassMethod *method, Scope *scope, MaximAst::PostfixExpression *expr) {
    std::vector<std::unique_ptr<Value>> tupleVals;
    tupleVals.reserve(expr->left->assignments.size());

    for (const auto &assignment : expr->left->assignments) {
        tupleVals.push_back(visitSinglePostfix(method, scope, expr, assignment.get()));
    }

    if (tupleVals.size() == 1) return std::move(tupleVals[0]);
    else
        return Tuple::create(method->moduleClass()->ctx(), std::move(tupleVals), method->builder(),
                             method->allocaBuilder(), expr->startPos, expr->endPos);
}
