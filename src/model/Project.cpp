#include "Project.h"

using namespace AxiomModel;

void Project::serialize(QDataStream &stream) const {
    library.serialize(stream);
    root.serialize(stream);
}

void Project::deserialize(QDataStream &stream) {
    library.deserialize(stream);
    root.deserialize(stream);
}
