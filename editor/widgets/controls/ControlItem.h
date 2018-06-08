#pragma once

#include <QtWidgets/QGraphicsObject>

#include "../IConnectable.h"
#include "common/Hookable.h"

namespace AxiomModel {
    class Control;
}

namespace AxiomGui {

    class NodeSurfaceCanvas;

    class ControlItem : public QGraphicsObject, public AxiomCommon::Hookable, public IConnectable {
    Q_OBJECT
        Q_PROPERTY(float hoverState
                       READ
                       hoverState
                       WRITE
                       setHoverState)

    public:
        AxiomModel::Control *control;

        NodeSurfaceCanvas *canvas;

        explicit ControlItem(AxiomModel::Control *control, NodeSurfaceCanvas *canvas);

        QRectF boundingRect() const override;

        QRectF aspectBoundingRect() const;

        bool isEditable() const;

        AxiomModel::Control *sink() override { return control; }

        float hoverState() const { return _hoverState; }

    public slots:

        void setHoverState(float hoverState);

    signals:

        void resizerPosChanged(QPointF newPos);

        void resizerSizeChanged(QSizeF newSize);

        void mouseEnter();

        void mouseLeave();

    protected:

        virtual bool showLabelInCenter() const = 0;

        QRectF drawBoundingRect() const;

        virtual QRectF useBoundingRect() const = 0;

        virtual QPainterPath controlPath() const = 0;

        QColor outlineNormalColor() const;

        QColor outlineActiveColor() const;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        void triggerGeometryChange();

        void triggerUpdate();

        void buildMenuStart(QMenu &menu);

        void buildMenuEnd(QMenu &menu);

        virtual void paintControl(QPainter *painter) = 0;

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void updateSelected(bool selected);

        void remove();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

        void resizerStartDrag();

        void resizerEndDrag();

    private:
        bool isMoving = false;
        bool isConnecting = false;
        QPointF mouseStartPoint;
        float _hoverState = 0;
        QRect startDragRect;

        QString getLabelText() const;
    };

}
