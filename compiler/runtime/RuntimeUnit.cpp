#include "RuntimeUnit.h"

#include <llvm/IR/Function.h>
#include <iostream>

#include "Runtime.h"

using namespace MaximRuntime;

RuntimeUnit::RuntimeUnit(Runtime *runtime) : _runtime(runtime) {

}

RuntimeUnit::~RuntimeUnit() = default;

void RuntimeUnit::setGetterMethod(std::unique_ptr<MaximCodegen::ComposableModuleClassMethod> method) {
    _method = std::move(method);
}

void RuntimeUnit::pullGetterMethod(MaximCodegen::ComposableModuleClassMethod *method) {
    auto mthd = method ? method : _method.get();
    if (mthd) {
        _getValueCb = (GetValueCb) _runtime->jit().getSymbolAddress(mthd->name());
        assert(_getValueCb);
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
    auto module = moduleClass();
    if (!module) {
        std::cout << "  Not saving: no module" << std::endl;
        return;
    }

    auto moduleType = module->storageType();
    auto size = _runtime->ctx()->dataLayout().getTypeAllocSize(moduleType);
    std::cout << "  Saving " << size << " bytes" << std::endl;
    _saveVal.type = moduleType;
    _saveVal.value = std::unique_ptr<void, MallocDeleter>(malloc(size));
    if (_saveVal.value) memcpy(_saveVal.value.get(), _currentPtr, size);
}

void RuntimeUnit::restoreValue() {
    if (!_saveVal.value) return;

    auto module = moduleClass();
    if (!module) return;

    auto moduleType = module->storageType();
    if (moduleType != _saveVal.type) return;

    auto size = _runtime->ctx()->dataLayout().getTypeAllocSize(_saveVal.type);
    std::cout << "  Restoring " << size << " bytes" << std::endl;
    memcpy(_currentPtr, _saveVal.value.get(), size);
}

void RuntimeUnit::setRestoreValue(MaximRuntime::RuntimeUnit::SaveValue &value) {
    _saveVal.type = value.type;

    if (value.value) {
        auto size = _runtime->ctx()->dataLayout().getTypeAllocSize(_saveVal.type);
        _saveVal.value = std::unique_ptr<void, MallocDeleter>(malloc(size));
        if (_saveVal.value) memcpy(_saveVal.value.get(), value.value.get(), size);
    } else _saveVal.value.reset();
}

void *RuntimeUnit::currentPtr() const {
    return _currentPtr;
}
