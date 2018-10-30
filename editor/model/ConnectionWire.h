#pragma once

#include <QtCore/QPointF>
#include <deque>

#include "WireGrid.h"
#include "common/Event.h"
#include "common/TrackedObject.h"

namespace AxiomModel {

    class GridSurface;

    class ConnectionWire : public AxiomCommon::TrackedObject {
    public:
        AxiomCommon::Event<const QPointF &> startPosChanged;
        AxiomCommon::Event<const QPointF &> endPosChanged;
        AxiomCommon::Event<const std::deque<QPoint> &, const std::vector<LineIndex> &> routeChanged;
        AxiomCommon::Event<bool> activeChanged;
        AxiomCommon::Event<bool> enabledChanged;
        AxiomCommon::Event<> removed;

        enum class WireType { NUM, MIDI };

        ConnectionWire(GridSurface *grid, WireGrid *wireGrid, WireType wireType, const QPointF &startPos,
                       const QPointF &endPos);

        ~ConnectionWire() override;

        GridSurface *grid() const { return _grid; }

        WireGrid *wireGrid() const { return _wireGrid; }

        WireType wireType() const { return _wireType; }

        const QPointF &startPos() const { return _startPos; }

        void setStartPos(const QPointF &startPos);

        const QPointF &endPos() const { return _endPos; }

        void setEndPos(const QPointF &endPos);

        const std::deque<QPoint> &route() const { return _route; }

        const std::vector<LineIndex> &lineIndices() const { return _lineIndices; }

        bool startActive() const { return _startActive; }

        void setStartActive(bool active);

        bool endActive() const { return _endActive; }

        void setEndActive(bool active);

        bool active() const { return _active; }

        bool startEnabled() const { return _startEnabled; }

        void setStartEnabled(bool enabled);

        bool endEnabled() const { return _endEnabled; }

        void setEndEnabled(bool enabled);

        bool enabled() const { return _enabled; }

        void remove();

    private:
        enum class ActiveState { NONE, START, END };

        GridSurface *_grid;
        WireGrid *_wireGrid;
        WireType _wireType;
        QPointF _startPos;
        QPointF _endPos;
        std::deque<QPoint> _route;
        std::vector<LineIndex> _lineIndices;
        ActiveState activeState = ActiveState::NONE;
        bool _startActive = false;
        bool _endActive = false;
        bool _active = false;
        bool _startEnabled = true;
        bool _endEnabled = true;
        bool _enabled = true;

        void updateRoute();

        void updateLineIndices();

        void updateActive();

        void updateEnabled();

        void setWireGrid(const std::deque<QPoint> &route);

        void clearWireGrid(const std::deque<QPoint> &route);

        std::vector<LineIndex> getLineIndices(const std::deque<QPoint> &route);
    };
}
