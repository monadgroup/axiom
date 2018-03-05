#include "NumberVisitor.h"

#include "../../ast/NumberExpression.h"
#include "../Num.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

std::unique_ptr<Value>
MaximCodegen::visitNumber(ComposableModuleClassMethod *method, Scope *scope, MaximAst::NumberExpression *expr) {
    return Num::create(method->moduleClass()->ctx(), expr->value, expr->value, MaximCommon::FormType::LINEAR, true,
                       expr->startPos, expr->endPos);
}
