#include <compiler/ast/VariableExpression.h>
#include "Scope.h"

#include "Value.h"
#include "Control.h"
#include "ComposableModuleClassMethod.h"
#include "MaximContext.h"
#include "Type.h"
#include "../common/CompileError.h"
#include "../ast/ControlExpression.h"

using namespace MaximCodegen;

std::unique_ptr<Value> Scope::getVariable(const std::string &name, SourcePos startPos, SourcePos endPos) {
    auto pos = _variables.find(name);
    if (pos == _variables.end()) return nullptr;
    return pos->second->withSource(startPos, endPos);
}

std::unique_ptr<Value> Scope::getControl(ComposableModuleClassMethod *method, MaximAst::ControlExpression *expr) {
    auto &controlDat = getControl(expr->name, expr->type, method);
    controlDat.isReadFrom = true;
    auto field = controlDat.control->getField(expr->prop);
    if (!field) {
        throw MaximCommon::CompileError(
            "so... uh... you... know that \" + expr->prop + \" isnt a valid property, right??",
            expr->startPos, expr->endPos
        );
    }

    auto val = method->callInto(controlDat.instId, {}, field->getValue(), expr->name + "." + expr->prop);
    return field->type()->createInstance(val, expr->startPos, expr->endPos);
}

void Scope::setVariable(std::string name, std::unique_ptr<Value> value) {
    auto pos = _variables.find(name);
    if (pos == _variables.end()) _variables.emplace(name, std::move(value));
    else pos->second = std::move(value);
}

void Scope::setControl(ComposableModuleClassMethod *method, MaximAst::ControlExpression *expr,
                       std::unique_ptr<Value> value) {
    auto &controlDat = getControl(expr->name, expr->type, method);
    controlDat.isWrittenTo = true;
    auto field = controlDat.control->getField(expr->prop);
    if (!field) {
        throw MaximCommon::CompileError(
            "so... uh... you... know that \" + expr->prop + \" isnt a valid property, right??",
            expr->startPos, expr->endPos
        );
    }

    method->moduleClass()->ctx()->assertType(value.get(), field->type());
    method->callInto(controlDat.instId, {value->get()}, field->setValue(), "");
}

void Scope::setAssignable(ComposableModuleClassMethod *method, MaximAst::AssignableExpression *assignable,
                          std::unique_ptr<Value> value) {
    if (auto var = dynamic_cast<MaximAst::VariableExpression*>(assignable)) {
        setVariable(var->name, std::move(value));
    } else {
        auto controlExpr = dynamic_cast<MaximAst::ControlExpression*>(assignable);
        assert(controlExpr);
        setControl(method, controlExpr, std::move(value));
    }
}

ControlInstance& Scope::getControl(const std::string &name, MaximCommon::ControlType type,
                                   ComposableModuleClassMethod *method) {
    ControlKey key = { name, type };
    auto pos = _controls.find(key);
    if (pos != _controls.end()) return pos->second;

    auto control = method->moduleClass()->ctx()->getControl(type);
    auto instId = method->moduleClass()->addEntry(control);
    auto newPos = _controls.emplace(key, ControlInstance { control, false, false, instId });
    return newPos.first->second;
}
