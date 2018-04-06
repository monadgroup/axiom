#include <iostream>
#include "Project.h"

#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

Project::Project(MaximRuntime::Runtime *runtime) : root(this, runtime->mainSurface()), _runtime(runtime) {
    build();
}

void Project::serialize(QDataStream &stream) const {
    library.serialize(stream);
    root.serialize(stream);
}

void Project::deserialize(QDataStream &stream) {
    library.deserialize(stream);
    root.deserialize(stream);
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
