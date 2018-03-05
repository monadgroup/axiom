#pragma once

#include "Control.h"
#include "../codegen/Scope.h"

namespace MaximCodegen {
    class ControlInstance;
}

namespace MaximRuntime {

    class HardControl : public Control {
    public:
        HardControl(Node *node, const std::string &name, const MaximCodegen::ControlInstance &instance);

        static std::unique_ptr<HardControl>
        create(Node *node, const std::string &name, const MaximCodegen::ControlInstance &instance);

        std::string name() const override { return _name; }

        MaximCodegen::Control *type() const override;

        bool writtenTo() const override;

        bool readFrom() const override;

        std::vector<Control *> internallyLinkedControls() override;

        const MaximCodegen::ControlInstance &instance() const { return _instance; }

        void setInstance(const MaximCodegen::ControlInstance &instance) { _instance = instance; }

    private:

        std::string _name;

        MaximCodegen::ControlInstance _instance;
    };

}
