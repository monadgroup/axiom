#include "ModuleRuntimeUnit.h"

#include "Runtime.h"
#include "../codegen/MaximContext.h"

using namespace MaximRuntime;

ModuleRuntimeUnit::ModuleRuntimeUnit(Runtime *runtime, const std::string &name)
    : RuntimeUnit(runtime, &_module), _module(name, runtime->ctx()->llvm()) {
}

void ModuleRuntimeUnit::reset() {
    _module = llvm::Module(_module.getName(), runtime()->ctx()->llvm());
}

void ModuleRuntimeUnit::deploy() {
    if (_isDeployed) runtime()->jit().removeModule(_deployKey);

    _deployKey = runtime()->jit().addModule(_module);
    _isDeployed = true;
}
