#pragma once

#include "Control.h"
#include "../common/ControlType.h"

namespace MaximRuntime {

    class IONode;

    class IOControl : public Control {
    public:
        explicit IOControl(IONode *node, MaximCommon::ControlType type, bool isRead, bool isWrite);

        std::string name() const override { return ""; }

        MaximCodegen::Control *type() const override { return _type; }

        MaximCommon::ControlType ioType() const { return _ioType; }

        bool writtenTo() const override { return _isWrite; }

        bool readFrom() const override { return _isRead; }

        void addInternallyLinkedControls(std::set<Control *> &controls) override { }

    private:
        MaximCommon::ControlType _ioType;
        MaximCodegen::Control *_type;
        bool _isRead;
        bool _isWrite;
    };

}
