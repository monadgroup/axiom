#include "HardControl.h"

#include "codegen/Control.h"

using namespace MaximRuntime;

HardControl::HardControl(Node *node, std::string name, MaximCodegen::Control *control)
    : Control(node, std::move(name), control->type()), _control(control) {

}

MaximCommon::ControlDirection HardControl::direction() const {
    return _control->direction;
}
