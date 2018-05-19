#pragma once

#include "common/Event.h"
#include "../grid/GridSurface.h"
#include "../ModelObject.h"

namespace MaximRuntime {
    class Surface;
}

namespace AxiomModel {

    class Node;

    class Control;

    class Connection;

    class NodeSurface : public ModelObject {
    public:
        using ChildCollection = WatchSequence<Node *>;
        using ConnectionCollection = WatchSequence<Connection *>;

        AxiomCommon::Event<const QString &> nameChanged;
        AxiomCommon::Event<const QPointF &> panChanged;
        AxiomCommon::Event<float> zoomChanged;

        NodeSurface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        static std::unique_ptr<NodeSurface>
        deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream, const QUuid &parent, bool withContext) const override;

        ChildCollection &nodes() { return _nodes; }

        const ChildCollection &nodes() const { return _nodes; }

        ConnectionCollection &connections() { return _connections; }

        const ConnectionCollection &connections() const { return _connections; }

        GridSurface &grid() { return _grid; }

        const GridSurface &grid() const { return _grid; }

        virtual QString name() = 0;

        virtual bool canExposeControl() const = 0;

        QPointF pan() const { return _pan; }

        void setPan(QPointF pan);

        float zoom() const { return _zoom; }

        void setZoom(float zoom);

        void attachRuntime(MaximRuntime::Surface *runtime);

        void doRuntimeUpdate();

        void saveValue();

        void restoreValue();

        void remove() override;

    private:
        ChildCollection _nodes;
        ConnectionCollection _connections;
        GridSurface _grid;
        QPointF _pan;
        float _zoom;

        std::optional<MaximRuntime::Surface *> _runtime;

        void nodeAdded(Node *node) const;
    };

}
