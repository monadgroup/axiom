#include <iostream>
#include "GroupNode.h"

#include "GeneratableModuleClass.h"
#include "SoftControl.h"
#include "Runtime.h"

using namespace MaximRuntime;

GroupNode::GroupNode(Surface *surface) : Node(surface), _subsurface(surface->runtime(), surface->depth() + 1) {

}

GeneratableModuleClass* GroupNode::compile() {
    auto result = _subsurface.compile();

    std::string type_str;
    llvm::raw_string_ostream rso(type_str);
    result->storageType()->print(rso);
    //std::cout << "GroupNode type: " << rso.str() << std::endl;

    for (auto &control : _controls) {
        auto targetGroup = control->forward()->group();
        auto targetIndex = _subsurface.groupPtrIndexes().find(targetGroup);
        assert(targetIndex != _subsurface.groupPtrIndexes().end());

        control->setInstanceId(targetIndex->second);
    }

    return result;
}

void GroupNode::forwardControl(MaximRuntime::Control *control) {
    assert(control->node()->surface() == &_subsurface);

    auto newControl = SoftControl::create(this, control);
    auto newControlPtr = newControl.get();
    _controls.push_back(std::move(newControl));
    emit controlAdded(newControlPtr);

    control->node()->scheduleCompile();
}

void GroupNode::pullGetterMethod(MaximCodegen::ComposableModuleClassMethod *method) {
    RuntimeUnit::pullGetterMethod(method);
    _subsurface.pullGetterMethod(method ? method : getterMethod());
}

void* GroupNode::updateCurrentPtr(void *parentCtx) {
    auto selfPtr = RuntimeUnit::updateCurrentPtr(parentCtx);
    _subsurface.updateCurrentPtr(parentCtx);
    return selfPtr;
}

void GroupNode::saveValue() {
    _subsurface.saveValue();
}

void GroupNode::restoreValue() {
    _subsurface.restoreValue();
}

void GroupNode::setRestoreValue(MaximRuntime::RuntimeUnit::SaveValue &value) {
    _subsurface.setRestoreValue(value);
}

MaximCodegen::ModuleClass* GroupNode::moduleClass() {
    return _subsurface.moduleClass();
}
