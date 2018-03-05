#include "UnaryVisitor.h"

#include "../../ast/UnaryExpression.h"
#include "../ComposableModuleClassMethod.h"
#include "../MaximContext.h"
#include "../Num.h"
#include "../Tuple.h"
#include "ExpressionVisitor.h"

using namespace MaximCodegen;

static std::unique_ptr<Num>
visitSingleUnary(ComposableModuleClassMethod *method, Scope *scope, MaximAst::UnaryExpression *expr,
                 std::unique_ptr<Value> val) {
    auto num = method->moduleClass()->ctx()->assertNum(std::move(val));

    switch (expr->type) {
        case MaximAst::UnaryExpression::Type::POSITIVE:
            return std::move(num);
        case MaximAst::UnaryExpression::Type::NEGATIVE: {
            auto numVec = num->vec(method->builder());
            auto negVec = method->builder().CreateFNeg(numVec, "negative");
            return num->withVec(method->builder(), negVec, expr->startPos, expr->endPos);
        }
        case MaximAst::UnaryExpression::Type::NOT: {
            auto numVec = num->vec(method->builder());
            auto zeroFloat = method->moduleClass()->ctx()->constFloat(0);
            auto zeroVec = llvm::ConstantVector::get({zeroFloat, zeroFloat});
            auto compareResult = method->builder().CreateFCmp(llvm::CmpInst::Predicate::FCMP_OEQ, numVec, zeroVec,
                                                              "equalzero");
            auto compareFloat = method->builder().CreateUIToFP(compareResult,
                                                               method->moduleClass()->ctx()->numType()->vecType());
            return num->withVec(method->builder(), compareFloat, expr->startPos, expr->endPos);
        }
    }

    assert(false);
    throw;
}

std::unique_ptr<Value>
MaximCodegen::visitUnary(ComposableModuleClassMethod *method, Scope *scope, MaximAst::UnaryExpression *expr) {
    auto exprVal = visitExpression(method, scope, expr->expr.get());

    if (auto tuple = dynamic_cast<Tuple *>(exprVal.get())) {
        Tuple::Storage tupleVals;
        tupleVals.reserve(tuple->type()->types().size());
        for (size_t i = 0; i < tuple->type()->types().size(); i++) {
            tupleVals.push_back(
                visitSingleUnary(method, scope, expr,
                                 tuple->atIndex(i, method->builder(), expr->startPos, expr->endPos)));
        }
        return Tuple::create(method->moduleClass()->ctx(), std::move(tupleVals), method->builder(), expr->startPos,
                             expr->endPos);
    }

    return visitSingleUnary(method, scope, expr, std::move(exprVal));
}
