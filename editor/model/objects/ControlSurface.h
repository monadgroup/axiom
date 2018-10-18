#pragma once

#include "../CachedSequence.h"
#include "../ModelObject.h"
#include "../ModelRoot.h"
#include "../PoolOperators.h"
#include "../grid/GridSurface.h"
#include "common/WatchSequence.h"

namespace AxiomModel {

    class Node;

    class Control;

    class ControlSurface : public ModelObject {
    public:
        using ChildCollection = CachedSequence<FindChildrenWatchSequence<ModelRoot::ControlCollection>>;

        AxiomCommon::Event<bool> controlsOnTopRowChanged;

        ControlSurface(const QUuid &uuid, const QUuid &parentUuid, AxiomModel::ModelRoot *root);

        static std::unique_ptr<ControlSurface> create(const QUuid &uuid, const QUuid &parentUuid,
                                                      AxiomModel::ModelRoot *root);

        static QPoint nodeToControl(QPoint p) { return p * 2; }

        static QSize nodeToControl(QSize s) { return s * 2; }

        static QPoint controlToNodeFloor(QPoint p) { return {(int) floorf(p.x() / 2.f), (int) floorf(p.y() / 2.f)}; }

        static QPoint controlToNodeCeil(QPoint p) { return {(int) ceilf(p.x() / 2.f), (int) ceilf(p.y() / 2.f)}; }

        static QPointF controlToNode(QPointF p) { return {p.x() / 2., p.y() / 2.}; }

        static QSize controlToNodeFloor(QSize s) {
            return {(int) floorf(s.width() / 2.f), (int) floorf(s.height() / 2.f)};
        }

        static QSize controlToNodeCeil(QSize s) {
            return {(int) ceilf(s.width() / 2.f), (int) ceilf(s.height() / 2.f)};
        }

        static QSizeF controlToNode(QSizeF p) { return {p.width() / 2., p.height() / 2.}; }

        Node *node() const { return _node; }

        ChildCollection &controls() { return _controls; }

        GridSurface &grid() { return _grid; }

        const GridSurface &grid() const { return _grid; }

        bool controlsOnTopRow() const { return _controlsOnTopRow; }

        void remove() override;

    private:
        Node *_node;
        ChildCollection _controls;
        GridSurface _grid;
        bool _controlsOnTopRow = false;

        void setSize(QSize size);
        void updateControlsOnTopRow();
    };
}
