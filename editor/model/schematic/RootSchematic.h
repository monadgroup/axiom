#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <cassert>

#include "Schematic.h"

namespace MaximRuntime {
    class RootSurface;
}

namespace AxiomModel {

    class IONode;

    class RootSchematic : public Schematic {
    Q_OBJECT

    public:
        explicit RootSchematic(Project *project);

        QString name() override;

        SurfaceRef ref() const override { return SurfaceRef(); }

        bool canExposeControl() override { return false; }

        void exposeControl(AxiomModel::NodeControl *control) override { assert(false); }

        void attachRuntime(MaximRuntime::RootSurface *surface);

    private:

        IONode *inputNode;
        IONode *outputNode;

    };

}
