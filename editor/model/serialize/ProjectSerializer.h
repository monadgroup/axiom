#pragma once

#include <QtCore/QDataStream>
#include <memory>

namespace AxiomModel {

    class Project;

    namespace ProjectSerializer {
        static constexpr uint32_t schemaVersion = 3;
        static constexpr uint32_t minSchemaVersion = 2;
        static constexpr uint64_t projectSchemaMagic = 0x4D4F4E4144415850; // "MONADAXP"
        static constexpr uint64_t librarySchemaMagic = 0x4D4F4E414441584C; // "MONADAXL"

        void writeHeader(QDataStream &stream, uint64_t magic);

        bool readHeader(QDataStream &stream, uint64_t expectedMagic, uint32_t *versionOut);

        void serialize(Project *project, QDataStream &stream);

        std::unique_ptr<Project> deserialize(QDataStream &stream, uint32_t *versionOut);
    }
}
