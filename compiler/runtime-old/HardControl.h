#pragma once

#include <memory>

#include "Control.h"

namespace MaximRuntime {

    class CustomNode;

    class HardControl : public Control {
    public:
        HardControl(CustomNode *node, std::string name, MaximCodegen::Control *control);

        static std::unique_ptr<HardControl> create(CustomNode *node, std::string name, MaximCodegen::Control *control);

        MaximCodegen::Control *control() const override { return _control; }

        MaximCommon::ControlDirection direction() const override;

        void setControl(MaximCodegen::Control *control) { _control = control; }

    private:
        MaximCodegen::Control *_control;

    };

}
