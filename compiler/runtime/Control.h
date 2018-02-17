#pragma once

#include <string>
#include <memory>
#include <vector>

#include "../common/ControlType.h"
#include "../common/ControlDirection.h"
#include "../codegen/Control.h"

namespace llvm {
    class Value;
}

namespace MaximRuntime {

    class Node;

    class ControlGroup;

    class ExposedControl;

    class Control {
    public:
        bool isNew = true;
        bool isDeleted = false;

        Control(Node *node, std::string name, MaximCodegen::Control *control);

        static std::unique_ptr<Control> create(Node *node, std::string name, MaximCodegen::Control *control);

        std::string name() const { return _name; }

        MaximCommon::ControlType type() const { return _type; }

        Node *node() const { return _node; }

        std::shared_ptr<ControlGroup> &group() { return _group; }

        MaximCommon::ControlDirection direction() const { return _dir; }

        void setDirection(MaximCommon::ControlDirection dir);

        void setGroup(std::shared_ptr<ControlGroup> &group) { _group = group; }

        std::vector<Control*> &connections() { return _connections; }

        ExposedControl *exposer() const { return _exposer; }

        void setExposer(ExposedControl *exposer);

        MaximCodegen::Control *control() const { return _control; }

        void setControl(MaximCodegen::Control *control) { _control = control; }

        void connectTo(Control *control);

        void disconnectFrom(Control *control);

        virtual void cleanup();

    private:
        Node *_node;
        std::string _name;
        MaximCommon::ControlType _type;
        MaximCommon::ControlDirection _dir;
        std::shared_ptr<ControlGroup> _group;
        std::vector<Control*> _connections;
        ExposedControl *_exposer = nullptr;
        MaximCodegen::Control *_control;
    };

}
