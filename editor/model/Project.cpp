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
    clear();

    library.deserialize(stream);
    root.deserialize(stream);
}

void Project::clear() {
    // todo: clear library

    root.selectAll();
    root.deleteSelectedItems();
}

void Project::build() {
    std::cout << "Saving value..." << std::endl;
    root.saveValue();
    _runtime->compile();
    std::cout << "Restoring value..." << std::endl;
    root.restoreValue();
    std::cout << "Finished project build" << std::endl;
}
