#include "RuntimeUnit.h"

#include <llvm/IR/Function.h>
#include <iostream>

#include "Runtime.h"

using namespace MaximRuntime;

RuntimeUnit::RuntimeUnit(Runtime *runtime) : _runtime(runtime) {

}

RuntimeUnit::~RuntimeUnit() = default;

void RuntimeUnit::setMethods(std::unique_ptr<MaximCodegen::ModuleClassMethod> getterMethod,
                             MaximCodegen::ModuleClassMethod *destructorMethod) {
    _getterMethod = std::move(getterMethod);
    _destructorMethod = destructorMethod;
}

void RuntimeUnit::pullMethods() {
    if (_getterMethod) {
        _getValueCb = (GetValueCb) _runtime->jit().getSymbolAddress(_getterMethod->name());
        assert(_getValueCb);
    }
    if (_destructorMethod) {
        _destructCb = (DestructCb) _runtime->jit().getSymbolAddress(_destructorMethod->name());
    }
}

void *RuntimeUnit::getValuePtr(void *parentCtx) {
    if (!_getValueCb || !parentCtx) return nullptr;
    return _getValueCb(parentCtx);
}

void *RuntimeUnit::updateCurrentPtr(void *parentCtx) {
    _currentPtr = getValuePtr(parentCtx);
    return _currentPtr;
}

void RuntimeUnit::saveValue() {
    if (!_currentPtr) return;
    auto module = moduleClass();
    if (!module) {
        std::cout << "  Not saving: no module" << std::endl;
        return;
    }

    auto moduleType = module->storageType();
    auto size = _runtime->ctx()->dataLayout().getTypeAllocSize(moduleType);
    std::cout << "  Saving " << size << " bytes" << std::endl;
    _saveType = moduleType;
    _saveValue = std::unique_ptr<void, ValueDeleter>(malloc(size));
    if (_saveValue) {
        memcpy(_saveValue.get(), _currentPtr, size);
    }
}

void RuntimeUnit::restoreValue() {
    if (!_saveValue || !_currentPtr) return;

    auto module = moduleClass();
    if (!module) return;

    auto moduleType = module->storageType();
    if (moduleType != _saveType) return;

    auto size = _runtime->ctx()->dataLayout().getTypeAllocSize(_saveType);
    std::cout << "  Restoring " << size << " bytes" << std::endl;
    memcpy(_currentPtr, _saveValue.get(), size);
    return;
}

void RuntimeUnit::destructIfNeeded() {
    auto module = moduleClass();

    if ((!module || module->storageType() != _saveType) && _destructCb && _currentPtr) {
        std::cout << "Invoking destructor because type has changed" << std::endl;
        _destructCb(_currentPtr);
    }
}

void RuntimeUnit::cleanup() {
    if (_currentPtr && _destructCb) {
        std::cout << "Invoking destructor on cleanup" << std::endl;
        _destructCb(_currentPtr);
    }
}

void *RuntimeUnit::currentPtr() const {
    return _currentPtr;
}
