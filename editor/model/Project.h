#pragma once

#include <QtCore/QString>
#include <memory>
#include <optional>

#include "common/Event.h"
#include "editor/backend/AudioConfiguration.h"
#include "objects/PortalControl.h"

namespace AxiomBackend {
    class DefaultConfiguration;
    class AudioBackend;
}

namespace AxiomModel {

    class RootSurface;
    class ModelRoot;

    class Project : public AxiomCommon::TrackedObject {
    public:
        AxiomCommon::Event<const QString &> linkedFileChanged;
        AxiomCommon::Event<bool> isDirtyChanged;

        explicit Project(const AxiomBackend::DefaultConfiguration &defaultConfiguration);

        Project(QString linkedFile, std::unique_ptr<ModelRoot> mainRoot);

        ~Project() override;

        ModelRoot &mainRoot() const { return *_mainRoot; }

        RootSurface *rootSurface() const { return _rootSurface; }

        const QString &linkedFile() const { return _linkedFile; }

        void setLinkedFile(QString linkedFile);

        const bool &isDirty() const { return _isDirty; }

        void setIsDirty(bool isDirty);

        void attachBackend(AxiomBackend::AudioBackend *backend) { _backend = backend; }

        AxiomBackend::AudioBackend *backend() const { return _backend; }

        static AxiomBackend::PortalType backendTypeFromControlType(PortalControl::PortalType);

        static AxiomBackend::PortalValue backendValueFromWireType(ConnectionWire::WireType);

        static PortalControl::PortalType controlTypeFromBackendType(AxiomBackend::PortalType);

        static ConnectionWire::WireType wireTypeFromBackendValue(AxiomBackend::PortalValue);

        AxiomBackend::AudioConfiguration getAudioConfiguration() const;

    private:
        std::unique_ptr<ModelRoot> _mainRoot;
        QString _linkedFile;
        bool _isDirty = false;

        AxiomBackend::AudioBackend *_backend = nullptr;
        RootSurface *_rootSurface;

        void addRootListeners();

        void rootModified();

        void rootConfigurationChanged();
    };
}
