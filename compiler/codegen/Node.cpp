#include "Node.h"

#include "MaximContext.h"
#include "Value.h"
#include "../ast/VariableExpression.h"

using namespace MaximCodegen;

Node::Node(MaximContext *ctx, llvm::Module *module) : _ctx(ctx), _builder(ctx->llvm()), _module(module) {
    _ctxType = llvm::StructType::create(ctx->llvm(), "nodectx");

    auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {_ctxType}, false);
    _func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "node", module);
    _nodeCtx = _func->arg_begin();

    auto funcBlock = llvm::BasicBlock::Create(_ctx->llvm(), "entry", _func);
    _builder.SetInsertPoint(funcBlock);
}

Value* Node::getVariable(std::string name) {
    auto pos = _variables.find(name);
    if (pos == _variables.end()) return nullptr;
    return pos->second.get();
}

void Node::setVariable(std::string name, std::unique_ptr<Value> value) {
    auto pos = _variables.find(name);
    if (pos == _variables.end()) _variables.emplace(name, std::move(value));
    else pos->second = std::move(value);
}

void Node::setAssignable(MaximAst::AssignableExpression *assignable, std::unique_ptr<Value> value) {
    if (auto var = dynamic_cast<MaximAst::VariableExpression*>(assignable)) {
        setVariable(var->name, std::move(value));
        return;
    }
    assert(false);
}

llvm::Value* Node::addInstantiable(std::unique_ptr<Instantiable> inst, Builder &b) {
    auto index = _instantiables.size();
    auto instType = inst->type();
    _instantiables.push_back(std::move(inst));
    return b.CreateStructGEP(instType, _nodeCtx, (unsigned int) index, "nodeinst");
}

void Node::complete() {
    std::vector<llvm::Type*> instTypes;
    for (const auto &inst : _instantiables) {
        instTypes.push_back(inst->type());
    }
    _ctxType->setBody(instTypes, false);
}

llvm::Constant* Node::instantiate() {
    assert(!_ctxType->isOpaque());

    std::vector<llvm::Constant*> instValues;
    for (const auto &inst : _instantiables) {
        instValues.push_back(inst->instantiate());
    }

    return llvm::ConstantStruct::get(_ctxType, instValues);
}
