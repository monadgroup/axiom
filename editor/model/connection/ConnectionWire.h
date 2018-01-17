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

    public slots:

        void remove();

        void updateRoute();

    signals:

        void routeChanged(const std::deque<QPoint> &route);

        void removed();

        void cleanup();

    private:
        std::deque<QPoint> m_route;
    };

}
