#pragma once

#include <memory>
#include <mutex>

#include "HistoryList.h"
#include "Pool.h"
#include "WatchSequence.h"
#include "editor/compiler/interface/Transaction.h"

namespace MaximCompiler {
    class Runtime;
}

namespace AxiomModel {

    class Project;

    class ModelObject;

    class NodeSurface;

    class Node;

    class ControlSurface;

    class Control;

    class Connection;

    class ReferenceMapper;

    class RootSurface;

    class ModelRoot : public AxiomCommon::Hookable {
    public:
        using NodeSurfaceCollection = WatchSequence<NodeSurface *>;
        using NodeCollection = WatchSequence<Node *>;
        using ControlSurfaceCollection = WatchSequence<ControlSurface *>;
        using ControlCollection = WatchSequence<Control *>;
        using ConnectionCollection = WatchSequence<Connection *>;

        AxiomCommon::Event<> modified;
        AxiomCommon::Event<> configurationChanged;

        ModelRoot();

        RootSurface *rootSurface() const;

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

        void attachRuntime(MaximCompiler::Runtime *runtime);

        MaximCompiler::Runtime *runtime() const { return _runtime; }

        std::lock_guard<std::mutex> lockRuntime();

        void applyItemsTo(const std::vector<QUuid> &items, MaximCompiler::Transaction *transaction);

        void applyCompile(const std::vector<QUuid> &items);

        void applyTransaction(MaximCompiler::Transaction transaction);

        void destroy();

    private:
        Pool _pool;
        HistoryList _history;
        NodeSurfaceCollection _nodeSurfaces;
        NodeCollection _nodes;
        ControlSurfaceCollection _controlSurfaces;
        ControlCollection _controls;
        ConnectionCollection _connections;

        std::mutex _runtimeLock;
        MaximCompiler::Runtime *_runtime = nullptr;
    };
}
