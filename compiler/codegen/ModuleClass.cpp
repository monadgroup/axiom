#include "ModuleClass.h"

#include "MaximContext.h"

using namespace MaximCodegen;

size_t ModuleClass::_nextIndex = 0;

ModuleClass::ModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name)
    : _index(_nextIndex++), _ctx(ctx), _module(module), _name(name) {

}

void ModuleClass::complete() {
    if (_completed) return;
    doComplete();
    _completed = true;
}

std::string ModuleClass::mangleMethodName(const std::string &name) {
    return "maximclass." + _name + "." + std::to_string(_index) + "." + name;
}

void ModuleClass::doComplete() {
    auto constructorMethod = constructor();
    if (constructorMethod) {
        constructorMethod->builder().CreateRetVoid();
    }
}

ZeroInitializedModuleClass::ZeroInitializedModuleClass(MaximContext *ctx, llvm::Module *module,
                                                       const std::string &name)
    : ModuleClass(ctx, module, name) {

}

llvm::Constant *ZeroInitializedModuleClass::initializeVal() {
    return llvm::ConstantAggregateZero::get(storageType());
}

UndefInitializedModuleClass::UndefInitializedModuleClass(MaximContext *ctx, llvm::Module *module,
                                                         const std::string &name)
    : ModuleClass(ctx, module, name) {

}

llvm::Constant *UndefInitializedModuleClass::initializeVal() {
    return llvm::UndefValue::get(storageType());
}

TypeInferencedModuleClass::TypeInferencedModuleClass(MaximContext *ctx, llvm::Module *module,
                                                     const std::string &name)
    : ModuleClass(ctx, module, name) {

}

llvm::Type *TypeInferencedModuleClass::storageType() {
    return initializeVal()->getType();
}

BasicZeroModuleClass::BasicZeroModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name,
                                           llvm::Type *type)
    : ZeroInitializedModuleClass(ctx, module, name), _storageType(type) {

}

