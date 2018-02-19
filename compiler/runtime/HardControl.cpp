#include "HardControl.h"

#include "codegen/Control.h"

using namespace MaximRuntime;

HardControl::HardControl(Node *node, std::string name, MaximCodegen::Control *control)
    : Control(node, std::move(name), control->type()), _control(control) {

}

std::unique_ptr<HardControl> HardControl::create(Node *node, std::string name, MaximCodegen::Control *control) {
    return std::make_unique<HardControl>(node, name, control);
}

MaximCommon::ControlDirection HardControl::direction() const {
    return _control->direction;
}
