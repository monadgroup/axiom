#pragma once

#include <memory>
#include <mutex>

#include "HistoryList.h"
#include "Pool.h"
#include "common/WatchSequence.h"
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

    class ModelRoot : public AxiomCommon::TrackedObject {
    public:
        template<class CollectionType>
        using ModelRootCollection = AxiomCommon::CastWatchSequence<CollectionType, Pool::Sequence>;

        using NodeSurfaceCollection = AxiomCommon::RefWatchSequence<ModelRootCollection<NodeSurface *>>;
        using NodeCollection = AxiomCommon::RefWatchSequence<ModelRootCollection<Node *>>;
        using ControlSurfaceCollection = AxiomCommon::RefWatchSequence<ModelRootCollection<ControlSurface *>>;
        using ControlCollection = AxiomCommon::RefWatchSequence<ModelRootCollection<Control *>>;
        using ConnectionCollection = AxiomCommon::RefWatchSequence<ModelRootCollection<Connection *>>;

        AxiomCommon::Event<> modified;
        AxiomCommon::Event<> configurationChanged;

        ModelRoot();

        RootSurface *rootSurface();

        Pool &pool() { return _pool; }

        const Pool &pool() const { return _pool; }

        HistoryList &history() { return _history; }

        const HistoryList &history() const { return _history; }

        NodeSurfaceCollection nodeSurfaces() { return AxiomCommon::refWatchSequence(&_nodeSurfaces); }

        NodeCollection nodes() { return AxiomCommon::refWatchSequence(&_nodes); }

        ControlSurfaceCollection controlSurfaces() { return AxiomCommon::refWatchSequence(&_controlSurfaces); }

        ControlCollection controls() { return AxiomCommon::refWatchSequence(&_controls); }

        ConnectionCollection connections() { return AxiomCommon::refWatchSequence(&_connections); }

        void attachRuntime(MaximCompiler::Runtime *runtime);

        MaximCompiler::Runtime *runtime() const { return _runtime; }

        std::lock_guard<std::mutex> lockRuntime();

        void setHistory(HistoryList history);

        void applyDirtyItemsTo(MaximCompiler::Transaction *transaction);

        void compileDirtyItems();

        void applyTransaction(MaximCompiler::Transaction transaction);

        void destroy();

    private:
        Pool _pool;
        HistoryList _history;
        ModelRootCollection<NodeSurface *> _nodeSurfaces;
        ModelRootCollection<Node *> _nodes;
        ModelRootCollection<ControlSurface *> _controlSurfaces;
        ModelRootCollection<Control *> _controls;
        ModelRootCollection<Connection *> _connections;

        std::mutex _runtimeLock;
        MaximCompiler::Runtime *_runtime = nullptr;
    };
}
