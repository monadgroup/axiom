#pragma once

#include "Node.h"
#include "OutputControl.h"

namespace MaximRuntime {

    class OutputNode : public Node {
    public:
        explicit OutputNode(Surface *surface);

        GeneratableModuleClass *compile() override;

        void remove() override;

        OutputControl *control() { return _control.get(); }

        const std::unique_ptr<Control> *begin() const override { return (std::unique_ptr<Control> *) &_control; }

        const std::unique_ptr<Control> *end() const override { return (std::unique_ptr<Control> *) &_control + 1; }

    private:

        std::unique_ptr<GeneratableModuleClass> _moduleClass;

        std::unique_ptr<OutputControl> _control;
    };

}
