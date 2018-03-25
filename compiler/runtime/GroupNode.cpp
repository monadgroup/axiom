#include <iostream>
#include "GroupNode.h"

#include "GeneratableModuleClass.h"
#include "SoftControl.h"
#include "Runtime.h"

using namespace MaximRuntime;

GroupNode::GroupNode(Surface *surface) : Node(surface), _subsurface(surface->runtime(), surface->depth() + 1) {

}

GeneratableModuleClass* GroupNode::compile() {
    return _subsurface.compile();
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
