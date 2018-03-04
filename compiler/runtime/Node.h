#pragma once

#include "RuntimeUnit.h"

namespace MaximRuntime {

    class Node : public RuntimeUnit {
    public:
        explicit Node(Surface *parent);

        Surface *surface() const { return _surface; }

        std::set<Control*> &controls() { return _controls; }

    private:
        Surface *_parent;
        std::set<Control*> _controls;
    };

}
