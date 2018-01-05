#pragma once

#include <QtCore/QObject>
#include <vector>
#include <memory>
#include <QtCore/QPoint>

#include "ConnectionWire.h"

namespace AxiomModel {

    class ConnectionSink;

    class GridSurface;

    class ConnectionSource : public QObject {
    Q_OBJECT

    public:
        GridSurface *surface;

        explicit ConnectionSource(GridSurface *surface);

        std::vector<std::unique_ptr<ConnectionWire>> const &outputs() const { return m_outputs; }

        QPoint pos() const { return m_pos; }

    public slots:

        void connectTo(ConnectionSink *sink);

        void setPos(QPoint pos);

    signals:

        void outputAdded(ConnectionWire *wire);

        void posChanged(QPoint newPos);

    private slots:

        void removeWire(ConnectionWire *wire);

    private:
        std::vector<std::unique_ptr<ConnectionWire>> m_outputs;

        QPoint m_pos;
    };

}
