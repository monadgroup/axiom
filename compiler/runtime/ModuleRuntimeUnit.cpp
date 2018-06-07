#include "ModuleRuntimeUnit.h"

#include "Runtime.h"

using namespace MaximRuntime;

ModuleRuntimeUnit::ModuleRuntimeUnit(Runtime *runtime, const std::string &name)
    : RuntimeUnit(runtime) {
    _module = createModule(name, runtime);
}

ModuleRuntimeUnit::~ModuleRuntimeUnit() {
    if (_isDeployed) runtime()->jit().markForRemove(_deployKey);
}

std::unique_ptr<llvm::Module> ModuleRuntimeUnit::createModule(const std::string &name, MaximRuntime::Runtime *runtime) {
    auto module = std::make_unique<llvm::Module>(name, runtime->ctx()->llvm());
    module->setDataLayout(runtime->ctx()->dataLayout());
    module->setTargetTriple(runtime->jit().targetMachine()->getTargetTriple().str());
    return std::move(module);
}

std::unique_ptr<llvm::Module> ModuleRuntimeUnit::reset() {
    return setModule(createModule(_module->getName(), runtime()));
}

std::unique_ptr<llvm::Module> ModuleRuntimeUnit::setModule(std::unique_ptr<llvm::Module> module) {
    auto oldModule = std::move(_module);
    _module = std::move(module);
    return oldModule;
}

void ModuleRuntimeUnit::deploy() {
    if (_isDeployed) runtime()->jit().removeModule(_deployKey);
    _deployKey = runtime()->jit().addModule(*_module);
    _isDeployed = true;
}
