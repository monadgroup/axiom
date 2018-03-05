#include "ModuleClassMethod.h"

#include "ModuleClass.h"
#include "MaximContext.h"

using namespace MaximCodegen;

ModuleClassMethod::ModuleClassMethod(ModuleClass *moduleClass, std::string name, llvm::Type *returnType,
                                     std::vector<llvm::Type *> paramTypes)
    : _moduleClass(moduleClass), _name(moduleClass->mangleMethodName(name)), _builder(moduleClass->ctx()->llvm()),
      _returnType(returnType ? returnType : llvm::Type::getVoidTy(moduleClass->ctx()->llvm())),
      _paramTypes(std::move(paramTypes)) {
    auto storageType = moduleClass->storageType();
    _paramTypes.push_back(llvm::PointerType::get(storageType, 0));

    auto func = get(moduleClass->module());
    _entryBlock = llvm::BasicBlock::Create(moduleClass->ctx()->llvm(), "entry", func);
    _builder.SetInsertPoint(_entryBlock);
    _contextPtr = func->arg_begin() + _paramTypes.size() - 1;
}

llvm::Function *ModuleClassMethod::get(llvm::Module *module) const {
    if (auto func = module->getFunction(_name)) {
        return func;
    }

    return llvm::Function::Create(
        llvm::FunctionType::get(_returnType, _paramTypes, false),
        llvm::Function::LinkageTypes::ExternalLinkage,
        _name, module
    );
}

llvm::Value *ModuleClassMethod::arg(size_t index) const {
    return get(_moduleClass->module())->arg_begin() + index;
}

llvm::Value *ModuleClassMethod::call(Builder &b, std::vector<llvm::Value *> args, llvm::Value *context,
                                     llvm::Module *module, const llvm::Twine &resultName) const {
    args.push_back(context);
    auto func = get(module);
    return CreateCall(b, func, args, resultName);
}
