#include "CompileUnit.h"

#include "../codegen/MaximContext.h"
#include "Runtime.h"

using namespace MaximRuntime;

CompileUnit::CompileUnit(Runtime *runtime)
    : CompileLeaf(runtime), _module("compileunit", runtime->context()->llvm()),
      _instFunc(runtime->context(), &_module) {
    _module.setDataLayout(runtime->context()->dataLayout());
}

CompileUnit::~CompileUnit() {
    if (_isDeployed) runtime()->jit()->removeModule(_deployKey);
}

void CompileUnit::scheduleCompile() {
    _needsCompile = true;
    _needsDeploy = true;
    if (parentUnit()) parentUnit()->scheduleCompile();
}

void CompileUnit::scheduleDeploy() {
    _needsDeploy = true;
    if (parentUnit()) parentUnit()->scheduleDeploy();
}

void CompileUnit::compile() {
    _needsCompile = false;
}

void CompileUnit::deploy() {
    if (_isDeployed) runtime()->jit()->removeModule(_deployKey);
    _deployKey = runtime()->jit()->addModule(_module);
    _isDeployed = true;
    _needsDeploy = false;

    CompileLeaf::deploy();
}
