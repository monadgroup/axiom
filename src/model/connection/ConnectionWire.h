#pragma once

#include <QtCore/QObject>

#include "../Grid.h"

namespace AxiomModel {

    class ConnectionSource;

    class ConnectionSink;

    class Schematic;

    class ConnectionWire : public QObject {
    Q_OBJECT

    public:
        Grid *grid;
        ConnectionSource *source;
        ConnectionSink *sink;

        ConnectionWire(Grid *grid, ConnectionSource *source, ConnectionSink *sink);

        const std::deque<QPoint> &route() const { return m_route; }

    public slots:

        void remove();

        void updateRoute();

    signals:

        void removed();

        void routeChanged(const std::deque<QPoint> &route);

    private:
        std::deque<QPoint> m_route;
    };

}
