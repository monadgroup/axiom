#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

#include "Schematic.h"

namespace AxiomModel {

    class ModuleNode;

    class ModuleSchematic : public Schematic {
    Q_OBJECT

    public:
        ModuleNode *node;

        explicit ModuleSchematic(ModuleNode *node);

        QString name() override;

        bool canExposeControl() override { return true; }

        void exposeControl(AxiomModel::NodeControl *control) override;
    };

}
