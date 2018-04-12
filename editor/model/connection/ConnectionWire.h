#pragma once

#include <QtCore/QObject>
#include <deque>

#include "ConnectionSink.h"

namespace AxiomModel {

    class GridSurface;

    class Schematic;

    class ConnectionWire : public QObject {
    Q_OBJECT

    public:
        Schematic *schematic;
        ConnectionSink *sinkA;
        ConnectionSink *sinkB;

        const ConnectionSink::Type type;

        ConnectionWire(Schematic *schematic, ConnectionSink *sinkA, ConnectionSink *sinkB);

        const std::deque<QPoint> &route() const { return m_route; }

        bool active() const { return m_active; }

    public slots:

        void remove();

        void removeNoOp();

        void updateRoute();

    signals:

        void routeChanged(const std::deque<QPoint> &route);

        void subPosChanged();

        void activeChanged(bool newActive);

        void removed();

        void cleanup();

    private slots:

        void updateActive();

    private:
        std::deque<QPoint> m_route;

        enum class ActiveState {
            NOTHING,
            SINK_A,
            SINK_B
        };
        ActiveState activeState = ActiveState::NOTHING;
        bool m_active = false;
    };

}
