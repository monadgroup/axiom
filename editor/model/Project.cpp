#include "Project.h"

#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

Project::Project(MaximRuntime::Runtime *runtime) : root(runtime->mainSurface()), _runtime(runtime) {

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
