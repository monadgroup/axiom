#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QGraphicsPathItem>
#include <deque>

#include "common/TrackedObject.h"
#include "editor/model/WireGrid.h"

namespace AxiomModel {
    class ConnectionWire;
}

namespace AxiomGui {

    class WireItem : public QObject, public QGraphicsPathItem, public AxiomCommon::TrackedObject {
        Q_OBJECT

    public:
        AxiomModel::ConnectionWire *wire;

        WireItem(QObject *parent, AxiomModel::ConnectionWire *wire);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    private slots:

        void triggerUpdate();

        void updateRoute(const std::deque<QPoint> &route, const std::vector<AxiomModel::LineIndex> &lineIndices);

        void setIsActive(bool active);

        void remove();
    };
}
