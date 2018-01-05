#pragma once

#include <QtCore/QObject>
#include <vector>

namespace AxiomModel {

    class ConnectionWire;

    class Schematic;

    class ConnectionSink : public QObject {
    Q_OBJECT

    public:
        Schematic *schematic;

        ConnectionSink(Schematic *schematic);

        std::vector<ConnectionWire *> const &inputs() const { return m_inputs; }

        void addWire(ConnectionWire *wire);

    signals:

        void inputAdded(ConnectionWire *wire);

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        std::vector<ConnectionWire *> m_inputs;
    };

}
