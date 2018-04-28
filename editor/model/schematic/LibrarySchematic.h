#pragma once

#include <cassert>

#include "Schematic.h"

namespace AxiomModel {

    class LibraryEntry;

    class LibrarySchematic : public Schematic {
    Q_OBJECT

    public:
        LibrarySchematic(LibraryEntry *entry, Project *project);

        QString name() override;

        SurfaceRef ref() const override;

        bool canExposeControl() override { return false; }

        void exposeControl(AxiomModel::NodeControl *control) { assert(false); }

    private:
        LibraryEntry *entry;
    };

}
