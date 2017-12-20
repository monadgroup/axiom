#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>

#include "Schematic.h"

namespace AxiomModel {

    class ModuleNode;

    class ModuleSchematic : public Schematic {
    public:
        ModuleNode *node;

        ModuleSchematic(ModuleNode *node);

        QString getName() override;
    };

}
