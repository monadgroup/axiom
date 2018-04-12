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

        QRectF aspectBoundingRect() const;

        bool isEditable() const;

        AxiomModel::NodeControl *sink() override;

    protected:

        QRectF drawBoundingRect() const;

        virtual QRectF useBoundingRect() const = 0;

        virtual QPainterPath controlPath() const = 0;

        virtual QColor outlineNormalColor() const = 0;

        virtual QColor outlineActiveColor() const = 0;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected slots:

        void triggerGeometryChange();

        void triggerUpdate();

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void updateSelected(bool selected);

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
