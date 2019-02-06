#include "Project.h"

#include "../backend/AudioBackend.h"
#include "../backend/AudioConfiguration.h"
#include "ModelRoot.h"
#include "PoolOperators.h"
#include "actions/CreatePortalNodeAction.h"
#include "objects/PortalNode.h"
#include "objects/RootSurface.h"

using namespace AxiomModel;

Project::Project(const AxiomBackend::DefaultConfiguration &defaultConfiguration)
    : _mainRoot(std::make_unique<ModelRoot>()) {
    addRootListeners();

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

        switch (portal.type) {
        case AxiomBackend::PortalType::INPUT:
            CreatePortalNodeAction::create(rootId, QPoint(-3, inputOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::INPUT, &mainRoot())
                ->forward(true);
            inputOffset += portalSpacing;
            break;
        case AxiomBackend::PortalType::OUTPUT:
            CreatePortalNodeAction::create(rootId, QPoint(3, outputOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::OUTPUT, &mainRoot())
                ->forward(true);
            outputOffset += portalSpacing;
            break;
        case AxiomBackend::PortalType::AUTOMATION:
            CreatePortalNodeAction::create(rootId, QPoint(0, automationOffset), QString::fromStdString(portal.name),
                                           wireType, PortalControl::PortalType::AUTOMATION, &mainRoot())
                ->forward(true);
            automationOffset += portalSpacing;
            break;
        }
    }
}

Project::Project(QString linkedFile, std::unique_ptr<AxiomModel::ModelRoot> mainRoot)
    : _mainRoot(std::move(mainRoot)), _linkedFile(std::move(linkedFile)), _rootSurface(_mainRoot->rootSurface()) {
    addRootListeners();
}

Project::~Project() {
    _mainRoot->destroy();
}

void Project::setLinkedFile(QString linkedFile) {
    if (linkedFile != _linkedFile) {
        _linkedFile = std::move(linkedFile);
        linkedFileChanged(_linkedFile);
    }
}

void Project::setIsDirty(bool isDirty) {
    if (isDirty != _isDirty) {
        _isDirty = isDirty;
        isDirtyChanged(isDirty);
    }
}

void Project::addRootListeners() {
    _mainRoot->modified.connectTo(this, &Project::rootModified);
    _mainRoot->configurationChanged.connectTo(this, &Project::rootConfigurationChanged);
}

void Project::rootModified() {
    if (!linkedFile().isEmpty() || !backend()->doesSaveInternally()) {
        setIsDirty(true);
    }
}

void Project::rootConfigurationChanged() {
    backend()->internalUpdateConfiguration();
}
