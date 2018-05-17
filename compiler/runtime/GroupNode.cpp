#include <iostream>
#include "GroupNode.h"

#include "Runtime.h"

using namespace MaximRuntime;

GroupNode::GroupNode(Surface *surface) : Node(surface), _subsurface(surface->runtime(), surface->depth() + 1) {

}

GeneratableModuleClass *GroupNode::compile() {
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

SoftControl *GroupNode::forwardControl(MaximRuntime::Control *control) {
    assert(control->node()->surface() == &_subsurface);

    auto newControl = SoftControl::create(this, control);
    auto newControlPtr = newControl.get();
    _controls.push_back(std::move(newControl));
    controlAdded.trigger(newControlPtr);

    control->node()->scheduleCompile();
    return newControlPtr;
}

void GroupNode::pullMethods(MaximCodegen::ModuleClassMethod *getterMethod,
                            MaximCodegen::ModuleClassMethod *destroyMethod) {
    RuntimeUnit::pullMethods(getterMethod, destroyMethod);
    _subsurface.pullMethods(getterMethod, destroyMethod);
}

void *GroupNode::updateCurrentPtr(void *parentCtx) {
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

MaximCodegen::ModuleClass *GroupNode::moduleClass() {
    return _subsurface.moduleClass();
}
