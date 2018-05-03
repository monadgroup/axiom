#pragma once

#include "../grid/GridSurface.h"
#include "../Event.h"
#include "../ModelObject.h"

namespace AxiomModel {

    class Node;

    class Surface : public ModelObject {
    public:
        using ChildCollection = CollectionView<Node*>;

        Event<const QString &> nameChanged;
        Event<const QPointF &> panChanged;
        Event<float> zoomChanged;

        Surface(const QUuid &uuid, const QUuid &parentUuid, QPointF pan, float zoom, AxiomModel::ModelRoot *root);

        std::unique_ptr<Surface> deserialize(QDataStream &stream, const QUuid &uuid, const QUuid &parentUuid, AxiomModel::ModelRoot *root);

        void serialize(QDataStream &stream) const override;

        ChildCollection &nodes() { return _nodes; }

        const ChildCollection &nodes() const { return _nodes; }

        GridSurface &grid() { return _grid; }

        const GridSurface &grid() const { return _grid; }

        virtual QString name() = 0;

        QPointF pan() const { return _pan; }

        void setPan(QPointF pan);

        float zoom() const { return _zoom; }

        void setZoom(float zoom);

    private:
        ChildCollection _nodes;
        GridSurface _grid;
        QPointF _pan;
        float _zoom;
    };

}
