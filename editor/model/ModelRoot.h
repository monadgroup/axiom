#pragma once

#include <QDataStream>
#include <memory>

#include "Pool.h"
#include "HistoryList.h"
#include "WatchSequence.h"

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
        using NodeSurfaceCollection = WatchSequence<NodeSurface *>;
        using NodeCollection = WatchSequence<Node *>;
        using ControlSurfaceCollection = WatchSequence<ControlSurface *>;
        using ControlCollection = WatchSequence<Control *>;
        using ConnectionCollection = WatchSequence<Connection *>;

        ModelRoot();

        explicit ModelRoot(QDataStream &stream);

        template<class T>
        static void serializeChunk(QDataStream &stream, const QUuid &parent, const T &objects) {
            stream << (uint32_t) objects.size();
            for (const auto &obj : objects) {
                QByteArray objectBuffer;
                QDataStream objectStream(&objectBuffer, QIODevice::WriteOnly);
                obj->serialize(objectStream, parent, true);
                stream << objectBuffer;
            }
        }

        void serialize(QDataStream &stream);

        Pool &pool() { return _pool; }

        const Pool &pool() const { return _pool; }

        HistoryList &history() { return _history; }

        const HistoryList &history() const { return _history; }

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

        void deserializeChunk(QDataStream &stream, const QUuid &parent);

        void destroy();

    private:
        Pool _pool;
        HistoryList _history;
        NodeSurfaceCollection _nodeSurfaces;
        NodeCollection _nodes;
        ControlSurfaceCollection _controlSurfaces;
        ControlCollection _controls;
        ConnectionCollection _connections;
    };

}
