#include "Node.h"

#include "MaximContext.h"
#include "Value.h"
#include "Num.h"
#include "visitors/ExpressionVisitor.h"
#include "../ast/VariableExpression.h"
#include "../ast/ControlExpression.h"
#include "../ast/Block.h"

#include "controls/NumberControl.h"

using namespace MaximCodegen;

Node::Node(MaximContext *ctx, llvm::Module *module) : InstantiableFunction(ctx, module) {
    auto undefPos = SourcePos(-1, -1);
    setVariable("PI", Num::create(ctx, M_PI, M_PI, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    setVariable("E", Num::create(ctx, M_E, M_E, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    setVariable("INFINITY", Num::create(ctx, FP_INFINITE, FP_INFINITE, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
}

void Node::generateCode(MaximAst::Block *block) {
    for (const auto &expr : block->expressions) {
        visitExpression(this, expr.get());
    }
}

std::unique_ptr<Value> Node::getVariable(const std::string &name, SourcePos startPos, SourcePos endPos) {
    auto pos = _variables.find(name);
    if (pos == _variables.end()) return nullptr;
    return pos->second->withSource(startPos, endPos);
}

std::unique_ptr<Value> Node::getControl(Builder &b, MaximAst::ControlExpression *expr) {
    auto controlDat = getControl(expr->name, expr->type, b);
    auto control = controlDat.control;
    if (!control->validateProperty(expr->prop)) {
        throw CodegenError(
            "so... uh... you... know that " + expr->prop + " isnt a valid property, right??",
            expr->startPos, expr->endPos
        );
    }
    control->direction = MaximCommon::ControlDirection::IN;
    return control->getProperty(b, expr->prop, controlDat.instPtr)->withSource(expr->startPos, expr->endPos);
}

void Node::setVariable(std::string name, std::unique_ptr<Value> value) {
    auto pos = _variables.find(name);
    if (pos == _variables.end()) _variables.emplace(name, std::move(value));
    else pos->second = std::move(value);
}

void Node::setControl(Builder &b, MaximAst::ControlExpression *expr,
                      std::unique_ptr<Value> value) {
    auto controlDat = getControl(expr->name, expr->type, b);
    auto control = controlDat.control;
    if (!control->validateProperty(expr->prop)) {
        throw CodegenError(
            "so... uh... you... know that " + expr->prop + " isnt a valid property, right??",
            expr->startPos, expr->endPos
        );
    }
    control->direction = MaximCommon::ControlDirection::OUT;
    control->setProperty(b, expr->prop, std::move(value), controlDat.instPtr);
}

void Node::setAssignable(Builder &b, MaximAst::AssignableExpression *assignable, std::unique_ptr<Value> value) {
    if (auto var = dynamic_cast<MaximAst::VariableExpression*>(assignable)) {
        setVariable(var->name, std::move(value));
        return;
    }

    auto controlExpr = dynamic_cast<MaximAst::ControlExpression*>(assignable);
    assert(controlExpr);
    setControl(b, controlExpr, std::move(value));
}

void Node::reset() {
    _variables.clear();
    _controls.clear();
    InstantiableFunction::reset();
}

Node::ControlValue &Node::getControl(std::string name, MaximCommon::ControlType type, Builder &b) {
    ControlKey key = {std::move(name), type};
    auto pair = _controls.find(key);

    if (pair == _controls.end()) {
        auto newControl = createControl(type);
        auto controlPtr = newControl.get();
        auto index = instantiables().size();
        auto ptr = addInstantiable(std::move(newControl));
        return _controls.emplace(key, ControlValue {controlPtr, ptr, index}).first->second;
    } else {
        return pair->second;
    }
}

std::unique_ptr<Control> Node::createControl(MaximCommon::ControlType type) {
    switch (type) {
        case MaximCommon::ControlType::NUMBER: return NumberControl::create(ctx());
        default: assert(false); throw;
    }
}
