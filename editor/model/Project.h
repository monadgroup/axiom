#pragma once

#include <optional>

#include "Library.h"
#include "ModelRoot.h"

namespace AxiomBackend {
    class AudioConfiguration;
}

namespace AxiomModel {

    class RootSurface;

    class Project {
    public:
        static constexpr uint32_t schemaVersion = 2;
        static constexpr uint64_t projectSchemaMagic = 0x4D4F4E4144415850;
        static constexpr uint64_t librarySchemaMagic = 0x4D4F4E414441584C;
        static constexpr uint32_t minSchemaVersion = 2;

        explicit Project(const AxiomBackend::AudioConfiguration &defaultConfiguration);

        ~Project();

        explicit Project(QDataStream &stream);

        static std::unique_ptr<Project> deserialize(QDataStream &stream, uint32_t *versionOut);

        static void writeHeader(QDataStream &stream, uint64_t magic);

        static bool readHeader(QDataStream &stream, uint64_t expectedMagic, uint32_t *versionOut);

        ModelRoot &mainRoot() { return _mainRoot; }

        const ModelRoot &mainRoot() const { return _mainRoot; }

        Library &library() { return _library; }

        const Library &library() const { return _library; }

        RootSurface *rootSurface() const { return _rootSurface; }

        void serialize(QDataStream &stream);

        void destroy();

    private:
        ModelRoot _mainRoot;
        Library _library;
        RootSurface *_rootSurface;
    };
}
