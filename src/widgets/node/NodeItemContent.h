#pragma once

#include <QtWidgets/QGraphicsObject>

namespace AxiomModel {
    class Node;
}

namespace AxiomGui {

    class NodeItemContent : public QGraphicsObject {
        Q_OBJECT

    public:
        AxiomModel::Node *node;

        explicit NodeItemContent(AxiomModel::Node *node);

        QRectF boundingRect() const override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    public slots:
        void triggerUpdate();

    protected:
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    private:
        bool isDragging;
        QPoint mouseStartPoint;
        QPoint nodeStartPoint;
    };

}
