#pragma once

#include <QtCore/QObject>

namespace AxiomModel {

    class ConnectionSource;

    class ConnectionSink;

    class ConnectionWire : public QObject {
    Q_OBJECT

    public:
        ConnectionSource *source;
        ConnectionSink *sink;

        ConnectionWire(ConnectionSource *source, ConnectionSink *sink);

    public slots:

        void updateRoute();

        void remove();

    signals:

        void removed();
    };

}
