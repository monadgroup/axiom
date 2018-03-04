#pragma once

#include "RuntimeUnit.h"

namespace MaximRuntime {

    class Node : public RuntimeUnit {
    public:
        explicit Node(Surface *parent);

        RuntimeUnit *parentUnit() override { return _parent; }

        std::set<Control*> &controls() { return _controls; }

        virtual std::vector<Control*, Control*> internallyLinkedControls();

    private:
        Surface *_parent;
        std::set<Control*> _controls;
    };

}
