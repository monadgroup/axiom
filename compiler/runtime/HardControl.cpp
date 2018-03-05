#include "HardControl.h"

#include "../codegen/Control.h"
#include "../codegen/Scope.h"

using namespace MaximRuntime;

HardControl::HardControl(Node *node, const std::string &name, const MaximCodegen::ControlInstance &instance)
    : Control(node), _name(name), _instance(instance) {
    finish();
}

std::unique_ptr<HardControl> HardControl::create(Node *node, const std::string &name,
                                                 const MaximCodegen::ControlInstance &instance) {
    return std::make_unique<HardControl>(node, name, instance);
}

MaximCodegen::Control *HardControl::type() const {
    return _instance.control;
}

bool HardControl::writtenTo() const {
    return _instance.isWrittenTo;
}

bool HardControl::readFrom() const {
    return _instance.isReadFrom;
}

std::vector<Control*> HardControl::internallyLinkedControls() {
    // hard controls can't be internally linked
    return {};
}
