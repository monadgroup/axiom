#pragma once

#include "Node.h"
#include "IOControl.h"

namespace MaximRuntime {

    class IONode : public Node {
    public:
        explicit IONode(Surface *surface, MaximCommon::ControlType type, bool isRead, bool isWrite);

        GeneratableModuleClass *compile() override;

        void remove() override;

        IOControl *control() { return _control.get(); }

        std::vector<Control*> controls() const override;

        MaximCodegen::ModuleClass *moduleClass() override;

    private:

        std::unique_ptr<GeneratableModuleClass> _moduleClass;

        std::unique_ptr<IOControl> _control;
    };

}
