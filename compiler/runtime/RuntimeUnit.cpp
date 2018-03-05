#include "RuntimeUnit.h"

#include <llvm/IR/Function.h>

#include "Runtime.h"

using namespace MaximRuntime;

RuntimeUnit::RuntimeUnit(Runtime *runtime) : _runtime(runtime) {

}

RuntimeUnit::~RuntimeUnit() = default;

void RuntimeUnit::setGetterMethod(std::unique_ptr<MaximCodegen::ComposableModuleClassMethod> method) {
    _method = std::move(method);
}

void RuntimeUnit::pullGetterMethod() {
    if (_method) {
        _getValueCb = (GetValueCb) _runtime->jit().getSymbolAddress(_method->name());
        assert(_getValueCb);
    }
}

void* RuntimeUnit::getValuePtr(void *parentCtx) {
    if (!_getValueCb || !parentCtx) return nullptr;
    return _getValueCb(parentCtx);
}

void* RuntimeUnit::updateCurrentPtr(void *parentCtx) {
    _currentPtr = getValuePtr(parentCtx);
    assert(_currentPtr);
    return _currentPtr;
}

void* RuntimeUnit::currentPtr() const {
    return _currentPtr;
}
