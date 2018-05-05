#pragma once

#include <QtCore/QPointF>
#include <deque>

#include "Hookable.h"
#include "Event.h"

namespace AxiomModel {

    class GridSurface;

    class ConnectionWire : public Hookable {
    public:
        Event<const QPoint &> startPosChanged;
        Event<const QPoint &> endPosChanged;
        Event<const std::deque<QPoint> &> routeChanged;

        ConnectionWire(const GridSurface *grid, const QPoint &startPos, const QPoint &endPos);

        const GridSurface *grid() const { return _grid; }

        const QPoint &startPos() const { return _startPos; }

        void setStartPos(const QPoint &startPos);

        const QPoint &endPos() const { return _endPos; }

        void setEndPos(const QPoint &endPos);

        const std::deque<QPoint> &route() const { return _route; }

    private:
        const GridSurface *_grid;
        QPoint _startPos;
        QPoint _endPos;
        std::deque<QPoint> _route;

        void updateRoute();
    };

}
