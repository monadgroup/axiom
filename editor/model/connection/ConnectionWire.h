#pragma once

#include <QtCore/QObject>
#include <deque>

namespace AxiomModel {

    class ConnectionSink;

    class GridSurface;

    class ConnectionWire : public QObject {
        Q_OBJECT

    public:
        GridSurface *surface;
        ConnectionSink *sinkA;
        ConnectionSink *sinkB;

        ConnectionWire(GridSurface *surface, ConnectionSink *sinkA, ConnectionSink *sinkB);

        const std::deque<QPoint> &route() const { return m_route; }

        bool active() const { return m_active; }

    public slots:

        void remove();

        void updateRoute();

    signals:

        void routeChanged(const std::deque<QPoint> &route);

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
