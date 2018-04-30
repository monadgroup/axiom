#pragma once

#include <QDataStream>
#include <memory>

namespace AxiomModel {

    class Pool;

    class ModelObject;

    class ModelRoot {
    public:
        static constexpr uint32_t schemaVersion = 2;
        static constexpr uint32_t schemaMagic = 0xDEFACED;
        static constexpr uint32_t minSchemaVersion = 2;

        explicit ModelRoot(Pool *pool);

        template<class T>
        static void serializeChunk(QDataStream &stream, const T &objects) {
            stream << schemaMagic;
            stream << schemaVersion;
            stream << (uint32_t) objects.size();

            for (const auto &obj : objects) {
                QByteArray objectBuffer;
                QDataStream objectStream(&objectBuffer, QIODevice::WriteOnly);
                obj->serialize(objectStream);
                stream << objectBuffer;
            }
        }

        Pool *pool() const { return _pool; }

        void deserializeChunk(QDataStream &stream, std::vector<std::unique_ptr<ModelObject>> &objects);

    private:
        Pool *_pool;
    };

}
