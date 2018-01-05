#pragma once

#include <QtCore/QObject>

namespace AxiomModel {

    class ConnectionSource;

    class ConnectionSink;

    class Schematic;

    class ConnectionWire : public QObject {
    Q_OBJECT

    public:
        ConnectionSource *source;
        ConnectionSink *sink;
        Schematic *schematic;

        ConnectionWire(Schematic *schematic, ConnectionSource *source, ConnectionSink *sink);

    public slots:

        void updateRoute();

        void remove();

    signals:

        void removed();
    };

}
