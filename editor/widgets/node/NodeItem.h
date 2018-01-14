#pragma once

#include <QtWidgets/QGraphicsObject>

namespace AxiomModel {
    class Node;

    class NodeControl;
}

namespace AxiomGui {

    class SchematicCanvas;

    class NodeItem : public QObject, public QGraphicsItemGroup {
    Q_OBJECT

    public:
        AxiomModel::Node *node;

        explicit NodeItem(AxiomModel::Node *node, SchematicCanvas *parent);

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void addControl(AxiomModel::NodeControl *control);

        void remove();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

        void resizerStartDrag();

        void triggerGeometryChange();

    signals:

        void resizerPosChanged(QPointF newPos);

        void resizerSizeChanged(QSizeF newSize);
    };

}
