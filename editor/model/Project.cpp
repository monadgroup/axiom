#include "Project.h"

#include "objects/RootSurface.h"

using namespace AxiomModel;

Project::Project() {
    // setup default project
    //  1. create default surface
    mainRoot().pool().registerObj(std::make_unique<AxiomModel::RootSurface>(QUuid::createUuid(), QPointF(0, 0), 0, &mainRoot()));

    //  2. add default inputs and outputs
    // todo
}

Project::Project(QDataStream &stream) : _mainRoot(stream) {
}

std::unique_ptr<Project> Project::deserialize(QDataStream &stream, uint32_t *versionOut) {
    if (!readHeader(stream, versionOut)) return nullptr;

    return std::make_unique<Project>(stream);
}

void Project::writeHeader(QDataStream &stream) {
    stream << schemaMagic;
    stream << schemaVersion;
}

bool Project::readHeader(QDataStream &stream, uint32_t *versionOut) {
    uint32_t magic; stream >> magic;
    if (magic != schemaMagic) return false;

    uint32_t version; stream >> version;
    if (versionOut) *versionOut = version;
    return version >= minSchemaVersion && version <= schemaVersion;

}

void Project::serialize(QDataStream &stream) {
    writeHeader(stream);
    _mainRoot.serialize(stream);
}

void Project::destroy() {
    _mainRoot.destroy();
}
