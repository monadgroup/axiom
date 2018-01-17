#pragma once

#include <QtWidgets/QGraphicsObject>

namespace AxiomModel {
    class NodeControl;
}

namespace AxiomGui {

    class ControlItem : public QGraphicsObject {
        Q_OBJECT

    public:
        AxiomModel::NodeControl *control;

        explicit ControlItem(AxiomModel::NodeControl *control);

        QRectF boundingRect() const override;

        bool isEditable() const;

    protected:

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    protected slots:

        void triggerGeometryChange();

        void triggerUpdate();

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void remove();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

        void resizerStartDrag();

    signals:

        void resizerPosChanged(QPointF newPos);

        void resizerSizeChanged(QSizeF newSize);

    private:
        bool isMoving;
        QPointF mouseStartPoint;
    };

}
