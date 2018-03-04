#pragma once

#include "RuntimeUnit.h"

namespace MaximRuntime {

    class Control : public RuntimeUnit {
    public:
        Control(Node *node, std::string name, MaximCommon::ControlType type);

        Node *node() const { return _node; }

        std::string name() const { return _name; }

        MaximCommon::ControlType type() const { return _type; }

        ControlGroup *group() const { return _group; }

        void setGroup(ControlGroup *group);

        virtual bool codeWritesTo() = 0;

        virtual bool codeReadsFrom() = 0;

        std::set<Control*> &connections();
    };

}
