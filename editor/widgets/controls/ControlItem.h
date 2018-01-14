#pragma once

#include <QtWidgets/QGraphicsItemGroup>

namespace AxiomModel {
    class NodeControl;
}

namespace AxiomGui {

    class NodeItem;

    class ControlItem : public QObject, public QGraphicsItemGroup {
        Q_OBJECT

    public:
        AxiomModel::NodeControl *control;
        NodeItem *parent;

        ControlItem(AxiomModel::NodeControl *control, NodeItem *parent);

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void remove();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

        void resizerStartDrag();

        void triggerGeometryChange();

    signals:

        void resizerPosChanged(QPointF newPos);

        void resizerSizeChanged(QSizeF newSize);
    };

}
