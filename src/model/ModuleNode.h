#pragma once
#include <memory>

#include "Node.h"

namespace AxiomModel {

    class ModuleSchematic;

    class ModuleNode : public Node {
    public:
        std::unique_ptr<ModuleSchematic> schematic;
    };

}
