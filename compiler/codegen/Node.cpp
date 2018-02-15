#include "Node.h"

#include <utility>

#include "MaximContext.h"
#include "Value.h"
#include "Num.h"
#include "../ast/VariableExpression.h"
#include "../ast/ControlExpression.h"

#include "controls/NumberControl.h"

using namespace MaximCodegen;

Node::Node(MaximContext *ctx, llvm::Module *module) : _ctx(ctx), _builder(ctx->llvm()), _module(module) {
    _ctxType = llvm::StructType::create(ctx->llvm(), "nodectx");

    auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::PointerType::get(_ctxType, 0)}, false);
    _func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "node", module);
    _nodeCtx = _func->arg_begin();

    auto funcBlock = llvm::BasicBlock::Create(_ctx->llvm(), "entry", _func);
    _builder.SetInsertPoint(funcBlock);

    auto undefPos = SourcePos(-1, -1);
    setVariable("PI", Num::create(ctx, M_PI, M_PI, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    setVariable("E", Num::create(ctx, M_E, M_E, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    setVariable("INFINITY", Num::create(ctx, FP_INFINITE, FP_INFINITE, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
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
    control->direction = Control::Direction::IN;
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
    control->direction = Control::Direction::OUT;
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

llvm::Value* Node::addInstantiable(std::unique_ptr<Instantiable> inst, Builder &b) {
    auto index = _instantiables.size();
    _instTypes.push_back(inst->type(ctx()));
    _instantiables.push_back(std::move(inst));

    // figure out type of struct so far to use in GEP
    auto currentStructType = llvm::StructType::get(_ctx->llvm(), _instTypes, false);
    auto typedCtx = b.CreatePointerCast(_nodeCtx, llvm::PointerType::get(currentStructType, 0));

    return b.CreateStructGEP(currentStructType, typedCtx, (unsigned int) index, "nodeinst");
}

void Node::complete() {
    _ctxType->setBody(_instTypes, false);
}

llvm::Constant* Node::getInitialVal(MaximContext *ctx) {
    assert(!_ctxType->isOpaque());

    std::vector<llvm::Constant*> instValues;
    for (const auto &inst : _instantiables) {
        instValues.push_back(inst->getInitialVal(ctx));
    }

    return llvm::ConstantStruct::get(_ctxType, instValues);
}

void Node::initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, Builder &b) {
    assert(!_ctxType->isOpaque());

    for (size_t i = 0; i < _instantiables.size(); i++) {
        auto itemPtr = b.CreateStructGEP(_ctxType, ptr, (unsigned int) i, "nodeinst");
        _instantiables[i]->initializeVal(ctx, module, itemPtr, b);
    }
}

Node::ControlValue &Node::getControl(std::string name, MaximCommon::ControlType type, Builder &b) {
    ControlKey key = {std::move(name), type};
    auto pair = _controls.find(key);

    if (pair == _controls.end()) {
        auto newControl = createControl(type);
        auto controlPtr = newControl.get();
        auto ptr = addInstantiable(std::move(newControl), b);
        return _controls.emplace(key, ControlValue {controlPtr, ptr}).first->second;
    } else {
        return pair->second;
    }
}

std::unique_ptr<Control> Node::createControl(MaximCommon::ControlType type) {
    switch (type) {
        case MaximCommon::ControlType::NUMBER: return NumberControl::create(_ctx);
        default: assert(false);
    }
}
