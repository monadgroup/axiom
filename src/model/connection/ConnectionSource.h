#pragma once

#include <QtCore/QObject>
#include <vector>
#include <memory>

namespace AxiomModel {

    class ConnectionWire;

    class ConnectionSink;

    class ConnectionSource : public QObject {
    Q_OBJECT

    public:
        std::vector<std::unique_ptr<ConnectionWire>> const &outputs() const { return m_outputs; }

    public slots:

        void connectTo(ConnectionSink *sink);

    signals:

        void outputAdded(ConnectionWire *wire);

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        std::vector<std::unique_ptr<ConnectionWire>> m_outputs;
    };

}
