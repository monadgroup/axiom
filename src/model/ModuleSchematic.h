#pragma once
#include "Schematic.h"

namespace AxiomModel {

    class ModuleNode;

    class ModuleSchematic : public Schematic {
    public:
        ModuleNode *node;

        ModuleSchematic(ModuleNode *node);

        std::string getName() override;
    };

}
