#include "ComposableModuleClass.h"

#include "MaximContext.h"
#include "ComposableModuleClassMethod.h"

using namespace MaximCodegen;

ComposableModuleClass::ComposableModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name, const std::vector<llvm::Type*> &constructorParams)
    : ModuleClass(ctx, module, name) {
    _constructor = std::make_unique<ComposableModuleClassMethod>(this, "constructor", nullptr, constructorParams);
}

ModuleClassMethod* ComposableModuleClass::constructor() {
    return _constructor.get();
}

llvm::Constant* ComposableModuleClass::initializeVal() {
    return llvm::ConstantStruct::get(storageType(), _defaultDict);
}

llvm::StructType* ComposableModuleClass::storageType() {
    return llvm::StructType::get(ctx()->llvm(), _typeDict);
}

size_t ComposableModuleClass::addEntry(llvm::Type *type) {
    assert(!completed());
    return addEntry(llvm::ConstantAggregateZero::get(type));
}

size_t ComposableModuleClass::addEntry(llvm::Constant *initValue) {
    assert(!completed());
    assert(_typeDict.size() == _defaultDict.size());

    auto index = _typeDict.size();
    _typeDict.push_back(initValue->getType());
    _defaultDict.push_back(initValue);
    return index;
}

size_t ComposableModuleClass::addEntry(ModuleClass *moduleClass, const std::vector<llvm::Value*> &constructorParams) {
    auto index = addEntry(moduleClass->initializeVal());
    auto moduleConstructor = moduleClass->constructor();
    if (moduleConstructor) {
        _constructor->callInto(index, constructorParams, moduleConstructor, "");
    }
    return index;
}

llvm::Value* ComposableModuleClass::getEntryPointer(Builder &b, size_t index, llvm::Value *context,
                                                    const llvm::Twine &resultName) {
    auto type = storageType();
    auto castedVal = b.CreateBitCast(context, llvm::PointerType::get(type, 0));
    return b.CreateStructGEP(type, castedVal, (unsigned int) index, resultName);
}

void ComposableModuleClass::doComplete() {
    for (const auto &module : _moduleClasses) {
        module->complete();
    }

    ModuleClass::doComplete();
}
