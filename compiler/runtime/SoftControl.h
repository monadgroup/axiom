#pragma once

#include "Control.h"

namespace MaximRuntime {

    class GroupNode;

    class SoftControl : public Control {
    public:
        Q_OBJECT

        SoftControl(GroupNode *node, Control *linkedControl);

        MaximCodegen::Control *control() const override { return nullptr; }

        MaximCommon::ControlDirection direction() const override;

        Control *linkedControl() const { return _linkedControl; }

    private:
        Control *_linkedControl;
    };

}
