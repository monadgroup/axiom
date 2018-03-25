#include <iostream>
#include "GroupNode.h"

#include "GeneratableModuleClass.h"
#include "SoftControl.h"
#include "Runtime.h"

using namespace MaximRuntime;

GroupNode::GroupNode(Surface *surface) : Node(surface), _subsurface(surface->runtime(), surface->depth() + 1) {

}

GeneratableModuleClass* GroupNode::compile() {
    if (!_class || _subsurface.needsGraphUpdate()) {
        reset();

        auto subsurfaceClass = _subsurface.compile();
        _class = std::make_unique<GeneratableModuleClass>(runtime()->ctx(), module(), "groupnode");
        auto index = _class->addEntry(subsurfaceClass);
        _class->generate()->callInto(index, {}, subsurfaceClass->generate(), "");
        _subsurface.setGetterMethod(_class->entryAccessor(index));

        _class->complete();
        deploy();
    }

    return _class.get();
}

void GroupNode::forwardControl(MaximRuntime::Control *control) {
    assert(control->node()->surface() == &_subsurface);

    auto newControl = SoftControl::create(this, control);
    auto newControlPtr = newControl.get();
    _controls.push_back(std::move(newControl));
    emit controlAdded(newControlPtr);

    scheduleCompile();
}

void GroupNode::pullGetterMethod() {
    RuntimeUnit::pullGetterMethod();
    _subsurface.pullGetterMethod();
}

void* GroupNode::updateCurrentPtr(void *parentCtx) {
    auto selfPtr = RuntimeUnit::updateCurrentPtr(parentCtx);
    _subsurface.updateCurrentPtr(selfPtr);
    return selfPtr;
}
