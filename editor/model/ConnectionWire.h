#pragma once

#include <QtCore/QPointF>
#include <deque>

#include "common/Hookable.h"
#include "common/Event.h"

namespace AxiomModel {

    class GridSurface;

    class ConnectionWire : public AxiomCommon::Hookable {
    public:
        AxiomCommon::Event<const QPointF &> startPosChanged;
        AxiomCommon::Event<const QPointF &> endPosChanged;
        AxiomCommon::Event<const std::deque<QPoint> &> routeChanged;
        AxiomCommon::Event<bool> activeChanged;
        AxiomCommon::Event<> removed;

        enum class WireType {
            NUM,
            MIDI
        };

        ConnectionWire(const GridSurface *grid, WireType wireType, const QPointF &startPos, const QPointF &endPos);

        const GridSurface *grid() const { return _grid; }

        WireType wireType() const { return _wireType; }

        const QPointF &startPos() const { return _startPos; }

        void setStartPos(const QPointF &startPos);

        const QPointF &endPos() const { return _endPos; }

        void setEndPos(const QPointF &endPos);

        const std::deque<QPoint> &route() const { return _route; }

        bool startActive() const { return _startActive; }

        void setStartActive(bool active);

        bool endActive() const { return _endActive; }

        void setEndActive(bool active);

        bool active() const { return _active; }

        void remove();

    private:
        enum class ActiveState {
            NONE,
            START,
            END
        };

        const GridSurface *_grid;
        WireType _wireType;
        QPointF _startPos;
        QPointF _endPos;
        std::deque<QPoint> _route;
        ActiveState activeState = ActiveState::NONE;
        bool _startActive = false;
        bool _endActive = false;
        bool _active = false;

        void updateRoute();

        void updateActive();
    };

}
