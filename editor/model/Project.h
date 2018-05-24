#pragma once

#include <optional>

#include "ModelRoot.h"
#include "Library.h"

namespace MaximRuntime {
    class Runtime;
}

namespace AxiomModel {

    class RootSurface;

    class Project {
    public:
        static constexpr uint32_t schemaVersion = 2;
        static constexpr uint64_t schemaMagic = 0x4D4F4E4144202020;
        static constexpr uint32_t minSchemaVersion = 2;

        Project();

        explicit Project(QDataStream &stream);

        static std::unique_ptr<Project> deserialize(QDataStream &stream, uint32_t *versionOut);

        static void writeHeader(QDataStream &stream);

        static bool readHeader(QDataStream &stream, uint32_t *versionOut);

        ModelRoot &mainRoot() { return _mainRoot; }

        const ModelRoot &mainRoot() const { return _mainRoot; }

        Library &library() { return _library; }

        const Library &library() const { return _library; }

        void serialize(QDataStream &stream);

        void attachRuntime(MaximRuntime::Runtime *runtime);

        void rebuild() const;

        void destroy();

    private:
        ModelRoot _mainRoot;
        Library _library;
        RootSurface *_rootSurface;
        std::optional<MaximRuntime::Runtime *> _runtime;

        void init();
    };

}
