#pragma once

#include "ModelRoot.h"

namespace AxiomModel {

    class Project {
    public:
        static constexpr uint32_t schemaVersion = 2;
        static constexpr uint32_t schemaMagic = 0xDEFACED;
        static constexpr uint32_t minSchemaVersion = 2;

        Project();

        explicit Project(QDataStream &stream);

        static std::unique_ptr<Project> deserialize(QDataStream &stream, uint32_t *versionOut);

        static void writeHeader(QDataStream &stream);

        static bool readHeader(QDataStream &stream, uint32_t *versionOut);

        ModelRoot &mainRoot() { return _mainRoot; }

        const ModelRoot &mainRoot() const { return _mainRoot; }

        void serialize(QDataStream &stream);

        void destroy();

    private:
        ModelRoot _mainRoot;
    };

}
