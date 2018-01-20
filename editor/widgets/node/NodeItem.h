#pragma once

#include <QtWidgets/QGraphicsObject>

class QGraphicsProxyWidget;

namespace AxiomModel {
    class Node;

    class NodeControl;
}

namespace AxiomGui {

    class NodePanel;

    class SchematicCanvas;

    class NodeItem : public QGraphicsObject {
    Q_OBJECT

    public:
        AxiomModel::Node *node;

        SchematicCanvas *canvas;

        NodeItem(AxiomModel::Node *node, SchematicCanvas *canvas);

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    public slots:

    protected:

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void addControl(AxiomModel::NodeControl *control);

        void setIsSelected(bool selected);

        void remove();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

        void resizerStartDrag();

        void triggerUpdate();

        void triggerGeometryChange();

    signals:

        void resizerPosChanged(QPointF newPos);

        void resizerSizeChanged(QSizeF newSize);

    private:
        //NodePanel *nodePanel;
        //QGraphicsProxyWidget *nodePanelProxy;
        bool isDragging = false;
        QPoint mouseStartPoint;

        void updateNodePanelPos(QPointF realNodePos);
    };

}
