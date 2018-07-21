#include "ProjectSerializer.h"

#include "../Library.h"
#include "../ModelRoot.h"
#include "../Project.h"
#include "LibrarySerializer.h"
#include "ModelObjectSerializer.h"

using namespace AxiomModel;

void ProjectSerializer::writeHeader(QDataStream &stream, uint64_t magic) {
    stream << static_cast<quint64>(magic);
    stream << schemaVersion;
}

bool ProjectSerializer::readHeader(QDataStream &stream, uint64_t expectedMagic, uint32_t *versionOut) {
    quint64 magic;
    stream >> magic;
    if (magic != expectedMagic) return false;

    uint32_t version;
    stream >> version;
    if (versionOut) *versionOut = version;
    return version >= minSchemaVersion && version <= schemaVersion;
}

void ProjectSerializer::serialize(AxiomModel::Project *project, QDataStream &stream) {
    writeHeader(stream, projectSchemaMagic);
    ModelObjectSerializer::serializeRoot(&project->mainRoot(), stream);
    LibrarySerializer::serialize(&project->library(), stream);
}

std::unique_ptr<Project> ProjectSerializer::deserialize(QDataStream &stream, uint32_t *versionOut) {
    uint32_t version;
    if (!readHeader(stream, projectSchemaMagic, &version)) {
        if (versionOut) *versionOut = version;
        return nullptr;
    }
    if (versionOut) *versionOut = version;

    auto project = std::make_unique<Project>();
    auto modelRoot = ModelObjectSerializer::deserializeRoot(stream, version, project.get());
    auto library = LibrarySerializer::deserialize(stream, version, project.get());
    project->init(std::move(modelRoot), std::move(library));
    return project;
}
