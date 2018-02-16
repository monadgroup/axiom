#include "ExposedControl.h"

using namespace MaximRuntime;

ExposedControl::ExposedControl(Node *node, Control *mainControl)
    : Control(node, mainControl->name(), nullptr), _mainControl(mainControl) {
    mainControl->setExposer(this);
}

void ExposedControl::cleanup() {
    _mainControl->setExposer(nullptr);
    Control::cleanup();
}
