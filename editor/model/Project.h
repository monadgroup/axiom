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

        static void writeHeader(QDataStream &stream);

        static bool readHeader(QDataStream &stream, uint32_t *versionOut);

        void serialize(QDataStream &stream);

    private:
        ModelRoot _mainRoot;

    };

}
