#pragma once

#include "NodeSurface.h"

namespace AxiomModel {

    class LibraryEntry;

    class ModuleSurface : public NodeSurface {
    public:
        ModuleSurface(const QUuid &uuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        QString name() override;

        QString debugName() override;

        bool canExposeControl() const override { return false; }

        bool canHavePortals() const override { return false; }

        uint64_t getRuntimeId() override { return 0; }

        void setEntry(AxiomModel::LibraryEntry *entry);

        AxiomModel::LibraryEntry *entry() const { return _entry; }

    private:
        AxiomModel::LibraryEntry *_entry = nullptr;
    };
}
