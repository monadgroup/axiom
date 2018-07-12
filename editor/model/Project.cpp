#include "Project.h"

#include "../backend/AudioConfiguration.h"
#include "PoolOperators.h"
#include "actions/CreatePortalNodeAction.h"
#include "objects/PortalNode.h"
#include "objects/RootSurface.h"

using namespace AxiomModel;

Project::Project(const AxiomBackend::AudioConfiguration &defaultConfiguration) : _mainRoot(this) {
    // setup default project
    //  1. create default surface
    auto rootSurface = std::make_unique<RootSurface>(QUuid(), QPointF(0, 0), 0, &mainRoot());
    _rootSurface = rootSurface.get();
    mainRoot().pool().registerObj(std::move(rootSurface));

    //  2. add default inputs and outputs
    int portalSpacing = 6;

    int inputCount = 0;
    int outputCount = 0;
    int automationCount = 0;
    for (const auto &portal : defaultConfiguration.portals) {
        switch (portal.type) {
        case AxiomBackend::PortalType::INPUT:
            inputCount++;
            break;
        case AxiomBackend::PortalType::OUTPUT:
            outputCount++;
            break;
        case AxiomBackend::PortalType::AUTOMATION:
            automationCount++;
            break;
        }
    }

    auto inputOffset = -(inputCount - 1) * portalSpacing / 2;
    auto outputOffset = -(outputCount - 1) * portalSpacing / 2;
    auto automationOffset = -(automationCount - 1) * portalSpacing / 2;
    for (const auto &portal : defaultConfiguration.portals) {
        ConnectionWire::WireType wireType;
        switch (portal.value) {
        case AxiomBackend::PortalValue::AUDIO:
            wireType = ConnectionWire::WireType::NUM;
            break;
        case AxiomBackend::PortalValue::MIDI:
            wireType = ConnectionWire::WireType::MIDI;
            break;
        }

        std::vector<QUuid> dummyItems;
        switch (portal.type) {
        case AxiomBackend::PortalType::INPUT:
            CreatePortalNodeAction::create(QUuid(), QPoint(-3, inputOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::INPUT, &mainRoot())
                ->forward(true, dummyItems);
            inputOffset += portalSpacing;
            break;
        case AxiomBackend::PortalType::OUTPUT:
            CreatePortalNodeAction::create(QUuid(), QPoint(3, outputOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::OUTPUT, &mainRoot())
                ->forward(true, dummyItems);
            outputOffset += portalSpacing;
            break;
        case AxiomBackend::PortalType::AUTOMATION:
            CreatePortalNodeAction::create(QUuid(), QPoint(0, automationOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::AUTOMATION, &mainRoot())
                ->forward(true, dummyItems);
            automationOffset += portalSpacing;
            break;
        }
    }
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
