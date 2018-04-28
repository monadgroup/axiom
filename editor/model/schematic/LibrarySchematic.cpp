#include "LibrarySchematic.h"

#include "../LibraryEntry.h"

using namespace AxiomModel;

LibrarySchematic::LibrarySchematic(LibraryEntry *entry, Project *project) : Schematic(project), entry(entry) {
    connect(entry, &LibraryEntry::nameChanged,
            this, &LibrarySchematic::nameChanged);
}

QString LibrarySchematic::name() {
    return entry->name();
}

SurfaceRef LibrarySchematic::ref() const {
    return SurfaceRef(entry->baseUuid());
}
