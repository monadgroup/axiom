#pragma once

#include "Control.h"

namespace MaximRuntime {

    class OutputNode;

    class OutputControl : public Control {
    public:
        explicit OutputControl(OutputNode *node);

        std::string name() const override { return ""; }

        MaximCodegen::Control *type() const override;

        bool writtenTo() const override { return false; }

        bool readFrom() const override { return true; }

        std::vector<Control*> internallyLinkedControls() override { return {}; }
    };

}
