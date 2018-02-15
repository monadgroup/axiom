#include "Node.h"

#include "MaximContext.h"
#include "Value.h"
#include "Num.h"
#include "../ast/VariableExpression.h"

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
