#pragma once

#include <QtCore/QDataStream>
#include <functional>
#include <memory>

namespace AxiomModel {

    class Project;

    namespace ProjectSerializer {
        // Schema version history:
        //  schemaVersion = 1 in 0.1.0
        //                = 2 in 0.2.0
        //                = 3 in 0.3.0
        //                = 4 in 0.3.2
        //                = 5 in 0.4.0
        static constexpr uint32_t schemaVersion = 5;
        static constexpr uint32_t minSchemaVersion = 2;
        static constexpr uint64_t projectSchemaMagic = 0x4D4F4E4144415850; // "MONADAXP"
        static constexpr uint64_t librarySchemaMagic = 0x4D4F4E414441584C; // "MONADAXL"

        void writeHeader(QDataStream &stream, uint64_t magic);

        bool readHeader(QDataStream &stream, uint64_t expectedMagic, uint32_t *versionOut);

        void serialize(Project *project, QDataStream &stream, std::function<void(QDataStream &)> writeLinkedFile);

        std::unique_ptr<Project> deserialize(QDataStream &stream, uint32_t *versionOut,
                                             std::function<QString(QDataStream &, uint32_t)> getLinkedFile);
    }
}
