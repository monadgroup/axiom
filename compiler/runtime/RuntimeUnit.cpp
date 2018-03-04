#include "RuntimeUnit.h"

using namespace MaximRuntime;

RuntimeUnit::RuntimeUnit(Runtime *runtime, llvm::Module *module) : _runtime(runtime), _module(module) {

}

void RuntimeUnit::updateGetter(llvm::Module *module) {
    // todo
}

void RuntimeUnit::scheduleCompile() {
    _needsCompile = true;
}

void* RuntimeUnit::getValuePtr(void *parentCtx) {
    // todo
}

void* RuntimeUnit::updateCurrentPtr(void *parentCtx) {
    // todo
}

void* RuntimeUnit::currentPtr() const {
    // todo
}
