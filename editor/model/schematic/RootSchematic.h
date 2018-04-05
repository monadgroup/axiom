#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <cassert>

#include "Schematic.h"

namespace MaximRuntime {
    class RootSurface;
}

namespace AxiomModel {

    class RootSchematic : public Schematic {
    Q_OBJECT

    public:
        explicit RootSchematic(Project *project, MaximRuntime::RootSurface *runtime);

        QString name() override;

        bool canExposeControl() override { return false; }

        void exposeControl(AxiomModel::NodeControl *control) override { assert(false); }

    };

}
