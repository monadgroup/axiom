#pragma once

#include "CompileUnit.h"
#include "Schematic.h"

namespace MaximRuntime {

    class Control;

    class Node : public CompileUnit {
    public:
        explicit Node(Schematic *parent);

        Schematic *parentUnit() const override { return _parent; }

        void compile() override = 0;

    private:
        Schematic *_parent;
    };

}
