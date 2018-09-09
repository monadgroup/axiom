#pragma once

#include <QtCore/QString>
#include <memory>
#include <optional>

#include "common/Event.h"

namespace AxiomBackend {
    class DefaultConfiguration;
    class AudioBackend;
}

namespace AxiomModel {

    class RootSurface;
    class Library;
    class ModelRoot;

    class Project : public AxiomCommon::Hookable {
    public:
        AxiomCommon::Event<const QString &> linkedFileChanged;
        AxiomCommon::Event<bool> isDirtyChanged;

        explicit Project(const AxiomBackend::DefaultConfiguration &defaultConfiguration);

        explicit Project(QString linkedFile);

        void init(std::unique_ptr<ModelRoot> mainRoot, std::unique_ptr<Library> library);

        ~Project() override;

        ModelRoot &mainRoot() const { return *_mainRoot; }

        Library &library() const { return *_library; }

        RootSurface *rootSurface() const { return _rootSurface; }

        const QString &linkedFile() const { return _linkedFile; }

        void setLinkedFile(QString linkedFile);

        const bool &isDirty() const { return _isDirty; }

        void setIsDirty(bool isDirty);

        void attachBackend(AxiomBackend::AudioBackend *backend) { _backend = backend; }

        AxiomBackend::AudioBackend *backend() const { return _backend; }

    private:
        std::unique_ptr<ModelRoot> _mainRoot;
        std::unique_ptr<Library> _library;
        QString _linkedFile;
        bool _isDirty = false;

        AxiomBackend::AudioBackend *_backend = nullptr;
        RootSurface *_rootSurface;
    };
}
