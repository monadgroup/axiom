#include "SoftControl.h"

using namespace MaximRuntime;

SoftControl::SoftControl(Node *node, Control *linkedControl)
    : Control(node, linkedControl->name(), linkedControl->type()), _linkedControl(linkedControl) {
    connect(linkedControl, &Control::removed,
            this, &SoftControl::remove);

    connectTo(linkedControl);
}

MaximCommon::ControlDirection SoftControl::direction() const {
    return _linkedControl->direction();
}
