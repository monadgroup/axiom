#include <iostream>

#include "Project.h"
#include "LibraryEntry.h"
#include "node/GroupNode.h"
#include "control/NodeControl.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

Project::Project(MaximRuntime::Runtime *runtime) : history(this), root(this, runtime->mainSurface()),
                                                   _runtime(runtime) {
    library.addEntry(std::make_unique<LibraryEntry>("LFO", std::set<QString>{}));
    library.addEntry(std::make_unique<LibraryEntry>("Bla", std::set<QString>{}));
    library.addEntry(std::make_unique<LibraryEntry>("Hello, world", std::set<QString>{}));
    library.addEntry(std::make_unique<LibraryEntry>("What's going on", std::set<QString>{}));
    library.addEntry(std::make_unique<LibraryEntry>("This is a test", std::set<QString>{}));
    library.addEntry(std::make_unique<LibraryEntry>("Test", std::set<QString>{}));
    library.addEntry(std::make_unique<LibraryEntry>("Hey there", std::set<QString>{}));
    library.addEntry(std::make_unique<LibraryEntry>("olololo", std::set<QString>{}));

    build();
}

void Project::serialize(QDataStream &stream) const {
    stream << schemaMagic;
    stream << schemaRevision;

    library.serialize(stream);
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

    library.deserialize(stream);
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
    library.clear();
    root.deleteAll();
}

void Project::build() {
    root.saveValue();
    _runtime->compile();
    root.restoreValue();
}

Schematic *Project::findSurface(const AxiomModel::SurfaceRef &ref) {
    Schematic *currentSurface = &root;

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
