#include "Project.h"

#include "../backend/AudioConfiguration.h"
#include "ModelRoot.h"
#include "PoolOperators.h"
#include "actions/CreatePortalNodeAction.h"
#include "objects/PortalNode.h"
#include "objects/RootSurface.h"

using namespace AxiomModel;

Project::Project(const AxiomBackend::DefaultConfiguration &defaultConfiguration)
    : _mainRoot(std::make_unique<ModelRoot>(this)) {
    // setup default project
    //  1. create default surface
    auto rootId = QUuid::createUuid();
    auto rootSurface = std::make_unique<RootSurface>(rootId, QPointF(0, 0), 0, 0, &mainRoot());
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
            CreatePortalNodeAction::create(rootId, QPoint(-3, inputOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::INPUT, &mainRoot())
                ->forward(true, dummyItems);
            inputOffset += portalSpacing;
            break;
        case AxiomBackend::PortalType::OUTPUT:
            CreatePortalNodeAction::create(rootId, QPoint(3, outputOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::OUTPUT, &mainRoot())
                ->forward(true, dummyItems);
            outputOffset += portalSpacing;
            break;
        case AxiomBackend::PortalType::AUTOMATION:
            CreatePortalNodeAction::create(rootId, QPoint(0, automationOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::AUTOMATION, &mainRoot())
                ->forward(true, dummyItems);
            automationOffset += portalSpacing;
            break;
        }
    }
}

Project::Project(QString linkedFile) : _linkedFile(std::move(linkedFile)) {}

void Project::init(std::unique_ptr<AxiomModel::ModelRoot> mainRoot) {
    _mainRoot = std::move(mainRoot);
    _rootSurface = _mainRoot->rootSurface();
}

Project::~Project() {
    _mainRoot->destroy();
}

void Project::setLinkedFile(QString linkedFile) {
    if (linkedFile != _linkedFile) {
        _linkedFile = std::move(linkedFile);
        linkedFileChanged.trigger(_linkedFile);
    }
}

void Project::setIsDirty(bool isDirty) {
    if (isDirty != _isDirty) {
        _isDirty = isDirty;
        isDirtyChanged.trigger(isDirty);
    }
}
