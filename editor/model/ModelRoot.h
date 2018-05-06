#pragma once

#include <QDataStream>
#include <memory>

#include "Pool.h"
#include "CollectionView.h"

namespace AxiomModel {

    class Pool;

    class ModelObject;

    class NodeSurface;

    class Node;

    class ControlSurface;

    class Control;

    class Connection;

    class ModelRoot {
    public:
        using NodeSurfaceCollection = CollectionView<NodeSurface*>;
        using NodeCollection = CollectionView<Node*>;
        using ControlSurfaceCollection = CollectionView<ControlSurface*>;
        using ControlCollection = CollectionView<Control*>;
        using ConnectionCollection = CollectionView<Connection*>;

        static constexpr uint32_t schemaVersion = 2;
        static constexpr uint32_t schemaMagic = 0xDEFACED;
        static constexpr uint32_t minSchemaVersion = 2;

        ModelRoot();

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

        Pool &pool() { return _pool; }

        const Pool &pool() const { return _pool; }

        NodeSurfaceCollection &nodeSurfaces() { return _nodeSurfaces; }

        const NodeSurfaceCollection &nodeSurfaces() const { return _nodeSurfaces; }

        NodeCollection &nodes() { return _nodes; }

        const NodeCollection &nodes() const { return _nodes; }

        ControlSurfaceCollection &controlSurfaces() { return _controlSurfaces; }

        const ControlSurfaceCollection &controlSurfaces() const { return _controlSurfaces; }

        ControlCollection &controls() { return _controls; }

        const ControlCollection &controls() const { return _controls; }

        ConnectionCollection &connections() { return _connections; }

        const ConnectionCollection &connections() const { return _connections; }

        void deserializeChunk(QDataStream &stream, const QUuid &parent, std::vector<std::unique_ptr<ModelObject>> &objects);

    private:
        Pool _pool;
        NodeSurfaceCollection _nodeSurfaces;
        NodeCollection  _nodes;
        ControlSurfaceCollection _controlSurfaces;
        ControlCollection  _controls;
        ConnectionCollection  _connections;
    };

}
