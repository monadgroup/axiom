#include <iostream>
#include "GroupNode.h"

#include "GeneratableModuleClass.h"
#include "SoftControl.h"

using namespace MaximRuntime;

GroupNode::GroupNode(Surface *surface) : Node(surface), _subsurface(surface->runtime(), surface->depth() + 1) {

}

GeneratableModuleClass* GroupNode::compile() {
    std::cout << "  << Compiling GroupNode surface" << std::endl;
    auto result = _subsurface.compile();
    std::cout << "  << Finished GroupNode surface compile" << std::endl;
    return result;
}

void GroupNode::forwardControl(MaximRuntime::Control *control) {
    assert(control->node()->surface() == &_subsurface);

    auto newControl = SoftControl::create(this, control);
    auto newControlPtr = newControl.get();
    _controls.push_back(std::move(newControl));

    emit controlAdded(newControlPtr);
}
