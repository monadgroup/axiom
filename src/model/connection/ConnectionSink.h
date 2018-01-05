#pragma once

#include <QtCore/QObject>
#include <vector>
#include <QtCore/QPoint>

namespace AxiomModel {

    class ConnectionWire;

    class Schematic;

    class ConnectionSink : public QObject {
    Q_OBJECT

    public:
        std::vector<ConnectionWire *> const &inputs() const { return m_inputs; }

        QPoint pos() const { return m_pos; }

        void addWire(ConnectionWire *wire);

    public slots:

        void setPos(QPoint pos);

    signals:

        void inputAdded(ConnectionWire *wire);

        void posChanged(QPoint newPos);

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        std::vector<ConnectionWire *> m_inputs;
        QPoint m_pos;
    };

}
