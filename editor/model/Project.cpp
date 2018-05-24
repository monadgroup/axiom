#include "Project.h"

#include "PoolOperators.h"
#include "objects/RootSurface.h"
#include "objects/PortalNode.h"
#include "actions/CreatePortalNodeAction.h"
#include "compiler/runtime/Runtime.h"

using namespace AxiomModel;

Project::Project() {
    init();

    // setup default project
    //  1. create default surface
    auto surfaceId = QUuid::createUuid();
    auto rootSurface = std::make_unique<RootSurface>(surfaceId, QPointF(0, 0), 0, &mainRoot());
    _rootSurface = rootSurface.get();
    mainRoot().pool().registerObj(std::move(rootSurface));

    //  2. add default inputs and outputs
    CreatePortalNodeAction::create(surfaceId, QPoint(-3, 0), "Keyboard", ConnectionWire::WireType::MIDI,
                                   PortalControl::PortalType::INPUT, &mainRoot())->forward(true);
    CreatePortalNodeAction::create(surfaceId, QPoint(3, 0), "Speakers", ConnectionWire::WireType::NUM,
                                   PortalControl::PortalType::OUTPUT, &mainRoot())->forward(true);
}

Project::Project(QDataStream &stream) : _mainRoot(stream), _library(stream) {
    init();

    auto rootSurfaces = findChildren(mainRoot().nodeSurfaces(), QUuid());
    assert(rootSurfaces.size() == 1);
    auto rootSurface = dynamic_cast<RootSurface*>(takeAt(rootSurfaces, 0));
    assert(rootSurface);

    _rootSurface = rootSurface;
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
    uint64_t magic;
    stream >> magic;
    if (magic != schemaMagic) return false;

    uint32_t version;
    stream >> version;
    if (versionOut) *versionOut = version;
    return version >= minSchemaVersion && version <= schemaVersion;

}

void Project::serialize(QDataStream &stream) {
    writeHeader(stream);
    _mainRoot.serialize(stream);
    _library.serialize(stream);
}

void Project::attachRuntime(MaximRuntime::Runtime *runtime) {
    assert(!_runtime);
    _runtime = runtime;
    _rootSurface->attachRuntime(runtime->mainSurface());

    // todo: make this not hardcoded!
    auto t = takeAt(_rootSurface->nodes(), 0);
    auto inputNode = dynamic_cast<PortalNode*>(t);
    assert(inputNode);
    inputNode->attachRuntime(runtime->mainSurface()->input);

    auto outputNode = dynamic_cast<PortalNode*>(takeAt(_rootSurface->nodes(), 1));
    assert(outputNode);
    outputNode->attachRuntime(runtime->mainSurface()->output);

    rebuild();
}

void Project::rebuild() const {
    _rootSurface->saveValue();
    if (_runtime) (*_runtime)->compile();
    _rootSurface->restoreValue();
}

void Project::destroy() {
    _mainRoot.destroy();
}

void Project::init() {
    _mainRoot.history().rebuildRequested.connect([this]() { rebuild(); });
}
