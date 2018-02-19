#include "HardControl.h"

#include "CustomNode.h"
#include "../codegen/Control.h"

using namespace MaximRuntime;

HardControl::HardControl(CustomNode *node, std::string name, MaximCodegen::Control *control)
    : Control(node, std::move(name), control->type()), _control(control) {

}

std::unique_ptr<HardControl> HardControl::create(CustomNode *node, std::string name, MaximCodegen::Control *control) {
    return std::make_unique<HardControl>(node, name, control);
}

MaximCommon::ControlDirection HardControl::direction() const {
    return _control->direction;
}
