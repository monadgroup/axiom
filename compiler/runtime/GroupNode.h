#pragma once

#include "Node.h"
#include "Schematic.h"

namespace MaximRuntime {

    class GroupNode : public Node {
    public:
        explicit GroupNode(Schematic *parent);

        Schematic &subsurface() const { return _subsurface; }

        void compile() override;

    private:

        Schematic _subsurface;

    };

}
