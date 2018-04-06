#include "ComposableModuleClass.h"

#include "MaximContext.h"
#include "ComposableModuleClassMethod.h"

using namespace MaximCodegen;

ComposableModuleClass::ComposableModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name,
                                             const std::vector<llvm::Type *> &constructorParams)
    : ModuleClass(ctx, module, name) {
    _constructor = std::make_unique<ComposableModuleClassMethod>(this, "constructor", nullptr, constructorParams);
    _destructor = std::make_unique<ComposableModuleClassMethod>(this, "destructor", nullptr);
}

ModuleClassMethod *ComposableModuleClass::constructor() {
    return _constructor.get();
}

ModuleClassMethod* ComposableModuleClass::destructor() {
    return _destructor.get();
}

std::unique_ptr<ComposableModuleClassMethod> ComposableModuleClass::entryAccessor(size_t index) {
    assert(index < _typeDict.size());
    auto method = std::make_unique<ComposableModuleClassMethod>(this, name() + "entry." + std::to_string(index),
                                                                llvm::PointerType::get(_typeDict[index], 0));
    method->builder().CreateRet(method->getEntryPointer(index, "entry"));
    return method;
}

llvm::StructType *ComposableModuleClass::storageType() {
    return llvm::StructType::get(ctx()->llvm(), _typeDict);
}

size_t ComposableModuleClass::addEntry(llvm::Type *type) {
    assert(!completed());

    auto index = _typeDict.size();
    _typeDict.push_back(type);
    return index;
}

size_t ComposableModuleClass::addEntry(ModuleClass *moduleClass, const std::vector<llvm::Value *> &constructorParams, bool callDefaults) {
    auto index = addEntry(moduleClass->storageType());
    auto moduleConstructor = moduleClass->constructor();
    if (callDefaults && moduleConstructor) {
        _constructor->callInto(index, constructorParams, moduleConstructor, "");
    }
    auto moduleDestructor = moduleClass->destructor();
    if (callDefaults && moduleDestructor) {
        _destructor->callInto(index, {}, moduleDestructor, "");
    }
    return index;
}

llvm::Value *ComposableModuleClass::getEntryPointer(Builder &b, size_t index, llvm::Value *context,
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
