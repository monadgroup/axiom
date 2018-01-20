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

        QPointF subPos() const { return m_subPos; }

        bool active() const { return m_active; }

    public slots:

        void addWire(ConnectionWire *wire);

        void setPos(QPoint pos, QPointF subPos);

        void setActive(bool active);

        void clearConnections();

    signals:

        void connectionAdded(ConnectionWire *wire);

        void connectionRemoved(ConnectionWire *wire);

        void posChanged(QPoint newPos);

        void subPosChanged(QPointF newSubPos);

        void activeChanged(bool newActive);

        void removed();

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        std::vector<ConnectionWire *> m_connections;
        QPoint m_pos;
        QPointF m_subPos;
        bool m_active = false;
    };

}
