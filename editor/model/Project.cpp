#include "Project.h"

#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

Project::Project(MaximRuntime::Runtime *runtime) : _runtime(runtime), root(&runtime->mainSurface()) {

}

void Project::serialize(QDataStream &stream) const {
    library.serialize(stream);
    root.serialize(stream);
}

void Project::deserialize(QDataStream &stream) {
    library.deserialize(stream);
    root.deserialize(stream);
}
