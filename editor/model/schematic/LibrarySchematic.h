#pragma once

#include <cassert>

#include "Schematic.h"

namespace AxiomModel {

    class LibrarySchematic : public Schematic {
    Q_OBJECT

    public:
        LibrarySchematic();

        QString name() override { return ""; }

        SurfaceRef ref() const override { return SurfaceRef(); }

        bool canExposeControl() override { return false; }

        void exposeControl(AxiomModel::NodeControl *control) { assert(false); }
    };

}
