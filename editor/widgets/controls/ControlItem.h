#pragma once

#include <QtWidgets/QGraphicsObject>

#include "../IConnectable.h"

namespace AxiomModel {
    class NodeControl;
}

namespace AxiomGui {

    class SchematicCanvas;

    class ControlItem : public QGraphicsObject, public IConnectable {
    Q_OBJECT

    public:
        AxiomModel::NodeControl *control;

        SchematicCanvas *canvas;

        explicit ControlItem(AxiomModel::NodeControl *control, SchematicCanvas *canvas);

        QRectF boundingRect() const override;

        bool isEditable() const;

        AxiomModel::ConnectionSink &sink() override;

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
        bool isMoving = false;
        bool isConnecting = false;
        QPointF mouseStartPoint;
    };

}
