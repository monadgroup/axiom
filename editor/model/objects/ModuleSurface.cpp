#include "ModuleSurface.h"

#include "../LibraryEntry.h"

using namespace AxiomModel;

ModuleSurface::ModuleSurface(const QUuid &uuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root)
    : NodeSurface(uuid, QUuid(), pan, zoom, root) {}

QString ModuleSurface::name() {
    return _entry->name();
}

QString ModuleSurface::debugName() {
    return "ModuleSurface";
}

void ModuleSurface::setEntry(AxiomModel::LibraryEntry *entry) {
    assert(!_entry);
    _entry = entry;
    _entry->nameChanged.connectTo(&nameChanged);
}
