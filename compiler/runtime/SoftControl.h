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

        Control *forward() const { return _forward; }

        bool writtenTo() const override;

        bool readFrom() const override;

        void setInstanceId(int64_t newId) { _instanceId = newId; }

        int64_t instanceId() const { return _instanceId; }

        void addInternallyLinkedControls(std::set<Control *> &controls) override;

    protected:

        void onRemove() override;

    private:

        GroupNode *_node;
        Control *_forward;

        int64_t _instanceId;
    };

}
