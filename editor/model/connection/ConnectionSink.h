#pragma once

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <vector>

namespace AxiomModel {

    class ConnectionWire;

    class GridSurface;

    class ConnectionSink : public QObject {
        Q_OBJECT

    public:
        std::vector<ConnectionWire *> const &connections() const { return m_connections; }

        QPoint pos() const { return m_pos; }

    public slots:

        void addWire(ConnectionWire *wire);

        void setPos(QPoint pos);

    signals:

        void connectionAdded(ConnectionWire *wire);

        void posChanged(QPoint newPos);

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        std::vector<ConnectionWire *> m_connections;
        QPoint m_pos;
    };

}
