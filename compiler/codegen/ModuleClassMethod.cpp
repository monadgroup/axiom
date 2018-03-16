#include "ModuleClassMethod.h"

#include "ModuleClass.h"
#include "MaximContext.h"

using namespace MaximCodegen;

ModuleClassMethod::ModuleClassMethod(ModuleClass *moduleClass, std::string name, llvm::Type *returnType,
                                     std::vector<llvm::Type *> paramTypes)
    : _moduleClass(moduleClass), _name(moduleClass->mangleMethodName(name)), _allocaBuilder(moduleClass->ctx()->llvm()),
      _builder(moduleClass->ctx()->llvm()),
      _returnType(returnType ? returnType : llvm::Type::getVoidTy(moduleClass->ctx()->llvm())),
      _paramTypes(std::move(paramTypes)) {
    _funcStorageType = llvm::PointerType::get(moduleClass->storageType(), 0);
    _paramTypes.push_back(_funcStorageType);

    auto func = get(moduleClass->module());
    auto allocaBlock = llvm::BasicBlock::Create(moduleClass->ctx()->llvm(), "alloca", func);
    _entryBlock = llvm::BasicBlock::Create(moduleClass->ctx()->llvm(), "entry", func);

    llvm::FastMathFlags mathFlags;
    mathFlags.setNoNaNs();
    mathFlags.setNoInfs();
    mathFlags.setNoSignedZeros();
    mathFlags.setAllowReciprocal();
    mathFlags.setAllowContract(true);
    mathFlags.setUnsafeAlgebra();

    _allocaBuilder.SetInsertPoint(allocaBlock);
    _allocaBuilder.setFastMathFlags(mathFlags);
    auto allocaEntryBr = _allocaBuilder.CreateBr(_entryBlock);
    _allocaBuilder.SetInsertPoint(allocaEntryBr);

    _builder.SetInsertPoint(_entryBlock);
    _builder.setFastMathFlags(mathFlags);
    _contextPtr = func->arg_begin() + _paramTypes.size() - 1;
}

llvm::Function *ModuleClassMethod::get(llvm::Module *module) const {
    if (auto func = module->getFunction(_name)) {
        return func;
    }

    auto func = llvm::Function::Create(
        llvm::FunctionType::get(_returnType, _paramTypes, false),
        llvm::Function::LinkageTypes::ExternalLinkage,
        _name, module
    );
    func->addFnAttr("denormal-fp-math", "positive-zero");
    return func;
}

llvm::Value *ModuleClassMethod::arg(size_t index) const {
    return get(_moduleClass->module())->arg_begin() + index;
}

llvm::Value *ModuleClassMethod::call(Builder &b, std::vector<llvm::Value *> args, llvm::Value *context,
                                     llvm::Module *module, const llvm::Twine &resultName) const {
    args.push_back(b.CreatePointerCast(context, _funcStorageType, "castctx"));
    auto func = get(module);
    return CreateCall(b, func, args, resultName);
}
