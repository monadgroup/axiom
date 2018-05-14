#include "Project.h"

#include "objects/RootSurface.h"
#include "actions/CreatePortalNodeAction.h"

using namespace AxiomModel;

Project::Project() {
    // setup default project
    //  1. create default surface
    auto surfaceId = QUuid::createUuid();
    mainRoot().pool().registerObj(std::make_unique<RootSurface>(surfaceId, QPointF(0, 0), 0, &mainRoot()));

    //  2. add default inputs and outputs
    CreatePortalNodeAction::create(surfaceId, QPoint(-3, 0), "Keyboard", ConnectionWire::WireType::MIDI, PortalControl::PortalType::INPUT, &mainRoot())->forward();
    CreatePortalNodeAction::create(surfaceId, QPoint(3, 0), "Speakers", ConnectionWire::WireType::NUM, PortalControl::PortalType::OUTPUT, &mainRoot())->forward();
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
