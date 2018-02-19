#pragma once

#include <memory>

#include "Control.h"

namespace MaximRuntime {

    class HardControl : public Control {
    public:
        HardControl(Node *node, std::string name, MaximCodegen::Control *control);

        static std::unique_ptr<HardControl> create(Node *node, std::string name, MaximCodegen::Control *control);

        MaximCodegen::Control *control() const override { return _control; }

        MaximCommon::ControlDirection direction() const override;

        void setControl(MaximCodegen::Control *control) { _control = control; }

        void forward();

    private:
        MaximCodegen::Control *_control;

    };

}
