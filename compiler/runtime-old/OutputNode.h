#pragma once

#include "Node.h"
#include "OutputControl.h"

namespace MaximRuntime {

    class OutputNode : public Node {
    public:
        explicit OutputNode(Schematic *parent);

        ~OutputNode() override;

        void compile() override;

        void remove() override;

        OutputControl *control() { return &_control; }

    private:

        OutputControl _control;

    };

}
