#pragma once

#include "Control.h"

namespace MaximRuntime {

    class GroupNode;

    class SoftControl : public Control {
    public:
        SoftControl(GroupNode *node, Control *forward);

        static std::unique_ptr<SoftControl>
        create(GroupNode *node, Control *forward);

        std::string name() const override;

        MaximCodegen::Control *type() const override;

        bool writtenTo() const override;

        bool readFrom() const override;

        void addInternallyLinkedControls(std::set<Control *> &controls) override;

    private:

        GroupNode *_node;
        Control *_forward;
    };

}
