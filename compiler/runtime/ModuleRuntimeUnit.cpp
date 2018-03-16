#include "ModuleRuntimeUnit.h"

#include "Runtime.h"

using namespace MaximRuntime;

ModuleRuntimeUnit::ModuleRuntimeUnit(Runtime *runtime, const std::string &name)
    : RuntimeUnit(runtime) {
    _module = std::make_unique<llvm::Module>(name, runtime->ctx()->llvm());
    _module->setDataLayout(runtime->ctx()->dataLayout());
}

std::unique_ptr<llvm::Module> ModuleRuntimeUnit::reset() {
    return setModule(std::make_unique<llvm::Module>(_module->getName(), runtime()->ctx()->llvm()));
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
