#pragma once
#include "Schematic.h"

namespace AxiomModel {

    class RootSchematic : public Schematic {
    public:
        std::string getName() override;
    };

}
