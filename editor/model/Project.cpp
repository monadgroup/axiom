#include "Project.h"

#include "PoolOperators.h"
#include "actions/CreatePortalNodeAction.h"
#include "objects/PortalNode.h"
#include "objects/RootSurface.h"

using namespace AxiomModel;

Project::Project() : _mainRoot(this) {
    // setup default project
    //  1. create default surface
    auto surfaceId = QUuid::createUuid();
    auto rootSurface = std::make_unique<RootSurface>(surfaceId, QPointF(0, 0), 0, &mainRoot());
    _rootSurface = rootSurface.get();
    mainRoot().pool().registerObj(std::move(rootSurface));

    //  2. add default inputs and outputs
    CreatePortalNodeAction::create(surfaceId, QPoint(-3, 0), "Keyboard", ConnectionWire::WireType::MIDI,
                                   PortalControl::PortalType::INPUT, &mainRoot())
        ->forward(true);
    CreatePortalNodeAction::create(surfaceId, QPoint(3, 0), "Speakers", ConnectionWire::WireType::NUM,
                                   PortalControl::PortalType::OUTPUT, &mainRoot())
        ->forward(true);
}

Project::Project(QDataStream &stream) : _mainRoot(this, stream), _library(this, stream) {
    auto rootSurfaces = findChildren(mainRoot().nodeSurfaces(), QUuid());
    assert(rootSurfaces.size() == 1);
    _rootSurface = dynamic_cast<RootSurface *>(takeAt(rootSurfaces, 0));
    assert(_rootSurface);
}

Project::~Project() {
    destroy();
}

std::unique_ptr<Project> Project::deserialize(QDataStream &stream, uint32_t *versionOut) {
    if (!readHeader(stream, projectSchemaMagic, versionOut)) return nullptr;

    return std::make_unique<Project>(stream);
}

void Project::writeHeader(QDataStream &stream, uint64_t magic) {
    stream << static_cast<quint64>(magic);
    stream << schemaVersion;
}

bool Project::readHeader(QDataStream &stream, uint64_t expectedMagic, uint32_t *versionOut) {
    quint64 magic;
    stream >> magic;
    if (magic != expectedMagic) return false;

    uint32_t version;
    stream >> version;
    if (versionOut) *versionOut = version;
    return version >= minSchemaVersion && version <= schemaVersion;
}

void Project::serialize(QDataStream &stream) {
    writeHeader(stream, projectSchemaMagic);
    _mainRoot.serialize(stream);
    _library.serialize(stream);
}

void Project::destroy() {
    _mainRoot.destroy();
}
