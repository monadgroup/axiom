#include <iostream>

#include "Project.h"
#include "LibraryEntry.h"
#include "node/GroupNode.h"
#include "control/NodeControl.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

Project::Project(MaximRuntime::Runtime *runtime) : history(this), library(this), root(this),
                                                   _runtime(runtime) {
    root.attachRuntime(runtime->mainSurface());
    build();
}

void Project::serialize(QDataStream &stream) const {
    stream << schemaMagic;
    stream << schemaRevision;

    //library.serialize(stream);
    root.serialize(stream);
    history.serialize(stream);
}

void Project::deserialize(QDataStream &stream) {
    quint32 readMagic;
    stream >> readMagic;
    if (readMagic != schemaMagic) throw DeserializeInvalidFileException();

    // todo: format upgrade system? maybe a file format that upgrades better?
    quint32 readRevision;
    stream >> readRevision;
    if (readRevision != schemaRevision) throw DeserializeInvalidSchemaException();

    //library.deserialize(stream);
    root.deserialize(stream);
    history.deserialize(stream);
}

void Project::load(QDataStream &stream) {
    clear();
    deserialize(stream);
    _runtime->compile();
    root.restoreValue();
}

void Project::clear() {
    //library.clear();
    root.deleteAll();
}

void Project::build() {
    root.saveValue();
    _runtime->compile();
    root.restoreValue();
}

Schematic *Project::findSurface(const AxiomModel::SurfaceRef &ref) {
    // base surface can either be the project root, or one of the library surfaces.
    // this is determined by the ref.root UUID, which is null for the project root, or a library entry
    // UUID for those.
    Schematic *currentSurface;
    if (ref.root.isNull()) currentSurface = &root;
    else {
        auto libraryEntry = library.findById(ref.root);
        if (!libraryEntry) return nullptr;
        currentSurface = &libraryEntry->schematic();
    }

    for (const auto &index : ref.path) {
        auto targetNode = dynamic_cast<GroupNode *>(currentSurface->items()[index].get());
        if (!targetNode) return nullptr;
        currentSurface = targetNode->schematic.get();
    }

    return currentSurface;
}

Node *Project::findNode(const AxiomModel::NodeRef &ref) {
    auto surface = findSurface(ref.surface);
    if (!surface) return nullptr;

    return dynamic_cast<Node *>(surface->items()[ref.index].get());
}

NodeControl *Project::findControl(const AxiomModel::ControlRef &ref) {
    auto node = findNode(ref.node);
    if (!node) return nullptr;

    return dynamic_cast<NodeControl *>(node->surface.items()[ref.index].get());
}
