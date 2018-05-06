#include "Project.h"

using namespace AxiomModel;

Project::Project() = default;

Project::Project(QDataStream &stream) : _mainRoot(stream) {
}

void Project::writeHeader(QDataStream &stream) {
    stream << schemaMagic;
    stream << schemaVersion;
}

bool Project::readHeader(QDataStream &stream, uint32_t *versionOut) {
    uint32_t magic; stream >> magic;
    if (magic != schemaMagic) return false;

    uint32_t version; stream >> version;
    if (version < minSchemaVersion || version > schemaVersion) return false;

    if (versionOut) *versionOut = version;
    return true;
}

void Project::serialize(QDataStream &stream) {
    _mainRoot.serialize(stream);
}
