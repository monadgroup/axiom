#pragma once

#include <QDataStream>
#include <memory>

#include "CollectionView.h"

namespace AxiomModel {

    class Pool;

    class ModelObject;

    class Surface;

    class Node;

    class Control;

    class ModelRoot {
    public:
        using SurfaceCollection = CollectionView<Surface*>;
        using NodeCollection = CollectionView<Node*>;
        using ControlCollection = CollectionView<Control*>;

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

        SurfaceCollection &surfaces() { return _surfaces; }

        const SurfaceCollection &surfaces() const { return _surfaces; }

        NodeCollection &nodes() { return _nodes; }

        const NodeCollection &nodes() const { return _nodes; }

        ControlCollection &controls() { return _controls; }

        const ControlCollection &controls() const { return _controls; }

        void deserializeChunk(QDataStream &stream, std::vector<std::unique_ptr<ModelObject>> &objects);

    private:
        Pool *_pool;
        SurfaceCollection _surfaces;
        NodeCollection  _nodes;
        ControlCollection  _controls;
    };

}
