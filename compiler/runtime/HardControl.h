#pragma once

#include "Control.h"
#include "../codegen/Scope.h"

namespace MaximCodegen {
    struct ControlInstance;
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

        void addInternallyLinkedControls(std::set<Control *> &controls) override {}

        int64_t instanceId() const override { return _instance.instId; }

        const MaximCodegen::ControlInstance &instance() const { return _instance; }

        void setInstance(const MaximCodegen::ControlInstance &instance) { _instance = instance; }

    private:

        std::string _name;

        MaximCodegen::ControlInstance _instance;
    };

}
