#pragma once

#include "Control.h"

namespace MaximRuntime {

    class ExposedControl : public Control {
    public:
        ExposedControl(Node *node, Control *mainControl);

        void cleanup() override;

    private:
        Control *_mainControl;
    };

}
