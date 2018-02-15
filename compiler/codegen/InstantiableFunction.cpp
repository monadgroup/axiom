#include "InstantiableFunction.h"

#include "MaximContext.h"

using namespace MaximCodegen;

InstantiableFunction::InstantiableFunction(MaximContext *ctx, llvm::Module *module)
    : _ctx(ctx), _builder(ctx->llvm()), _module(module) {
    _ctxType = llvm::StructType::create(ctx->llvm(), "nodectx");

    auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx->llvm()), {llvm::PointerType::get(_ctxType, 0)}, false);
    _func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "generate", module);
    _nodeCtx = _func->arg_begin();

    auto funcBlock = llvm::BasicBlock::Create(_ctx->llvm(), "entry", _func);
    _builder.SetInsertPoint(funcBlock);
}

llvm::Value* InstantiableFunction::addInstantiable(std::unique_ptr<Instantiable> inst, Builder &b) {
    auto index = _instantiables.size();
    _instTypes.push_back(inst->type(ctx()));
    _instantiables.push_back(std::move(inst));

    // cast nodeCtx to struct of what we have so far, since it's opaque at this point
    auto currentStructType = llvm::StructType::get(_ctx->llvm(), _instTypes, false);
    auto typedCtx = b.CreatePointerCast(_nodeCtx, llvm::PointerType::get(currentStructType, 0));

    return b.CreateStructGEP(currentStructType, typedCtx, (unsigned int) index, "inst");
}

void InstantiableFunction::complete() {
    _ctxType->setBody(_instTypes, false);
}

llvm::Constant* InstantiableFunction::getInitialVal(MaximContext *ctx) {
    assert(!_ctxType->isOpaque());

    std::vector<llvm::Constant*> instValues;
    for (const auto &inst : _instantiables) {
        instValues.push_back(inst->getInitialVal(ctx));
    }

    return llvm::ConstantStruct::get(_ctxType, instValues);
}

void InstantiableFunction::initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, Builder &b) {
    assert(!_ctxType->isOpaque());

    for (size_t i = 0; i < _instantiables.size(); i++) {
        auto itemPtr = b.CreateStructGEP(_ctxType, ptr, (unsigned int) i, "inst");
        _instantiables[i]->initializeVal(ctx, module, itemPtr, b);
    }
}
