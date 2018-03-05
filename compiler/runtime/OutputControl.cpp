#include "OutputControl.h"

#include "OutputNode.h"
#include "Runtime.h"

using namespace MaximRuntime;

OutputControl::OutputControl(OutputNode *node) : Control(node) {
    finish();
}

MaximCodegen::Control *OutputControl::type() const {
    return node()->runtime()->ctx()->getControl(MaximCommon::ControlType::NUMBER);
}
