#pragma once

#include "Node.h"

namespace MaximRuntime {

    class Surface;

    class Control;

    class GeneratableModuleClass;

    class GroupNode : public Node {
    public:
        explicit GroupNode(Surface *surface);

        GeneratableModuleClass *compile() override { assert(false); throw; }

        const std::unique_ptr<Control> *begin() const override { assert(false); throw; }

        const std::unique_ptr<Control> *end() const override { assert(false); throw; }

        Surface *subsurface() const { return nullptr; }
    };

}
