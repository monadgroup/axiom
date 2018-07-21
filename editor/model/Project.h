#pragma once

#include <memory>
#include <optional>

namespace AxiomBackend {
    class AudioConfiguration;
}

namespace AxiomModel {

    class RootSurface;
    class Library;
    class ModelRoot;

    class Project {
    public:
        explicit Project(const AxiomBackend::AudioConfiguration &defaultConfiguration);

        Project();

        void init(std::unique_ptr<ModelRoot> mainRoot, std::unique_ptr<Library> library);

        ~Project();

        ModelRoot &mainRoot() const { return *_mainRoot; }

        Library &library() const { return *_library; }

        RootSurface *rootSurface() const { return _rootSurface; }

    private:
        std::unique_ptr<ModelRoot> _mainRoot;
        std::unique_ptr<Library> _library;

        RootSurface *_rootSurface;
    };
}
