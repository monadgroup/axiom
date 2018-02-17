#include "InstantiableFunction.h"

#include "MaximContext.h"

using namespace MaximCodegen;

size_t InstantiableFunction::_nextId = 0;

InstantiableFunction::InstantiableFunction(MaximContext *ctx, llvm::Module *module)
    : _id(_nextId++), _ctx(ctx), _builder(ctx->llvm()), _initBuilder(ctx->llvm()), _module(module) {
    reset();
}

std::unique_ptr<InstantiableFunction> InstantiableFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<InstantiableFunction>(ctx, module);
}

llvm::Function* InstantiableFunction::generateFunc(llvm::Module *module) {
    auto funcName = "generate." + std::to_string(_id);
    auto existingFunc = module->getFunction(funcName);
    if (existingFunc) return existingFunc;

    auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(_ctx->llvm()), {llvm::PointerType::get(_ctxType, 0)}, false);
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module);
}

llvm::Function* InstantiableFunction::initializeFunc(llvm::Module *module) {
    auto funcName = "initialize." + std::to_string(_id);
    auto existingFunc = module->getFunction(funcName);
    if (existingFunc) return existingFunc;

    auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(_ctx->llvm()), {llvm::PointerType::get(_ctxType, 0)}, false);
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module);
}

llvm::Value* InstantiableFunction::addInstantiable(std::unique_ptr<Instantiable> inst) {
    auto instPtr = inst.get();
    _ownedInstantiables.push_back(std::move(inst));
    return addInstantiable(instPtr);
}

llvm::Value* InstantiableFunction::addInstantiable(Instantiable *inst) {
    auto index = _instantiables.size();
    _instTypes.push_back(inst->type(ctx()));
    _instantiables.push_back(inst);

    // cast nodeCtx to struct of what we have so far, since it's opaque at this point
    auto currentStructType = llvm::StructType::get(_ctx->llvm(), _instTypes, false);
    auto typedCtx = _builder.CreatePointerCast(_generateFunc->arg_begin(), llvm::PointerType::get(currentStructType, 0));

    return _builder.CreateStructGEP(currentStructType, typedCtx, (unsigned int) index, "inst");
}

void InstantiableFunction::complete() {
    _ctxType->setBody(_instTypes, false);


    for (size_t i = 0; i < _instantiables.size(); i++) {
        auto itemPtr = _initBuilder.CreateStructGEP(_ctxType, _initializeFunc->arg_begin(), (unsigned int) i, "inst");
        _instantiables[i]->initializeVal(_ctx, _module, itemPtr, _initBuilder);
    }

    _builder.CreateRetVoid();
    _initBuilder.CreateRetVoid();
}

void InstantiableFunction::reset() {
    _ctxType = llvm::StructType::create(_ctx->llvm(), "nodectx");

    if (_generateFunc) {
        _generateFunc->removeFromParent();
    }
    if (_initializeFunc) {
        _initializeFunc->removeFromParent();
    }

    _generateFunc = generateFunc(_module);
    auto genBlock = llvm::BasicBlock::Create(_ctx->llvm(), "entry", _generateFunc);
    _builder.SetInsertPoint(genBlock);

    _initializeFunc = initializeFunc(_module);
    auto initBlock = llvm::BasicBlock::Create(_ctx->llvm(), "entry", _initializeFunc);
    _initBuilder.SetInsertPoint(initBlock);
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
    CreateCall(b, initializeFunc(module), {ptr}, "");
}
