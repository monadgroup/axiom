#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QGraphicsPathItem>
#include <deque>

namespace AxiomModel {
    class ConnectionWire;
}

namespace AxiomGui {

    class WireItem : public QObject, public QGraphicsPathItem {
    Q_OBJECT

    public:
        AxiomModel::ConnectionWire *wire;

        explicit WireItem(AxiomModel::ConnectionWire *wire);

    private slots:

        void updateRoute();

        void setActive(bool active);

        void remove();

    };

}
