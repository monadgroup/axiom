#include <llvm/Support/raw_ostream.h>
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

    auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(_ctx->llvm()), {_ctx->voidPointerType()}, false);
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module);
}

llvm::Function* InstantiableFunction::initializeFunc(llvm::Module *module) {
    auto funcName = "initialize." + std::to_string(_id);
    auto existingFunc = module->getFunction(funcName);
    if (existingFunc) return existingFunc;

    auto funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(_ctx->llvm()), {_ctx->voidPointerType()}, false);
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

    auto currentType = type(_ctx);
    auto typedCtx = _builder.CreatePointerCast(_generateFunc->arg_begin(), llvm::PointerType::get(currentType, 0));

    return _builder.CreateStructGEP(currentType, typedCtx, (unsigned int) index, "inst");
}

llvm::Value* InstantiableFunction::getInitializePointer(Instantiable *inst) {
    std::vector<llvm::Value*> indexList;
    indexList.push_back(_ctx->constInt(64, 0, false));
    assert(getInstIndex(inst, indexList));

    auto typedCtx = _initBuilder.CreatePointerCast(_initializeFunc->arg_begin(), llvm::PointerType::get(type(_ctx), 0));

    return _initBuilder.CreateGEP(typedCtx, indexList, "inst");
}

llvm::Value* InstantiableFunction::getGeneratePointer(Instantiable *inst) {
    std::vector<llvm::Value*> indexList;
    indexList.push_back(_ctx->constInt(64, 0, false));
    assert(getInstIndex(inst, indexList));

    auto typedCtx = _builder.CreatePointerCast(_generateFunc->arg_begin(), llvm::PointerType::get(type(_ctx), 0));

    return _builder.CreateGEP(typedCtx, indexList, "inst");
}

void InstantiableFunction::complete() {
    auto t = type(_ctx);

    for (size_t i = 0; i < _instantiables.size(); i++) {
        auto typedCtx = _initBuilder.CreatePointerCast(_initializeFunc->arg_begin(), llvm::PointerType::get(t, 0));
        auto itemPtr = _initBuilder.CreateStructGEP(t, typedCtx, (unsigned int) i, "inst");
        _instantiables[i]->initializeVal(_ctx, _module, itemPtr, this, _initBuilder);
    }

    _builder.CreateRetVoid();
    _initBuilder.CreateRetVoid();
}

void InstantiableFunction::reset() {
    _instTypes.clear();
    _ownedInstantiables.clear();
    _instantiables.clear();

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
    std::vector<llvm::Constant*> instValues;
    for (const auto &inst : _instantiables) {
        instValues.push_back(inst->getInitialVal(ctx));
    }

    return llvm::ConstantStruct::get(type(ctx), instValues);
}

llvm::StructType* InstantiableFunction::type(MaximContext *ctx) const {
    return llvm::StructType::get(ctx->llvm(), _instTypes);
}

void InstantiableFunction::initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, InstantiableFunction *func, Builder &b) {
    CreateCall(b, initializeFunc(module), {ptr}, "");
}

bool InstantiableFunction::getInstIndex(Instantiable *inst, std::vector<llvm::Value*> &indexes) {
    for (size_t i = 0; i < _instantiables.size(); i++) {
        if (_instantiables[i] == inst) {
            indexes.push_back(_ctx->constInt(32, i, false));
            return true;
        }
    }

    std::vector<llvm::Value*> beforeIndexes(indexes);

    for (size_t i = 0; i < _instantiables.size(); i++) {
        auto indexedInst = _instantiables[i];
        if (auto indexedInstFunc = dynamic_cast<InstantiableFunction*>(indexedInst)) {
            indexes = beforeIndexes;
            indexes.push_back(_ctx->constInt(32, i, false));
            if (indexedInstFunc->getInstIndex(inst, indexes)) return true;
        }
    }

    return false;
}
