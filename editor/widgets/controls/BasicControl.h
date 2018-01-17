#pragma once

#include <QtWidgets/QGraphicsObject>

namespace AxiomModel {
    class NodeValueControl;
}

namespace AxiomGui {

    class NodeItem;

    class BasicControl : public QGraphicsObject {
    Q_OBJECT
        Q_PROPERTY(float hoverState
                           READ
                                   hoverState
                           WRITE
                           setHoverState)

    public:
        enum class BasicMode {
            PLUG,
            KNOB,
            SLIDER_H,
            SLIDER_V
        };

        AxiomModel::NodeValueControl *control;
        NodeItem *parent;

        BasicControl(AxiomModel::NodeValueControl *control, NodeItem *parent);

        QRectF boundingRect() const override;

        QRectF aspectBoundingRect() const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

        BasicMode mode() const;

        float hoverState() const { return m_hoverState; }

    public slots:

        void setHoverState(float newHoverState);

    signals:

        void mouseEnter();

        void mouseLeave();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void remove();

        void resizerChanged(QPointF topLeft, QPointF bottomRight);

        void resizerStartDrag();

        void triggerGeometryChange();

        void triggerUpdate();

    signals:

        void resizerPosChanged(QPointF newPos);

        void resizerSizeChanged(QSizeF newSize);

    private:
        float m_hoverState = 0;
        bool isMoving;
        bool isDragging;
        float beforeDragVal;
        QPointF mouseStartPoint;

        QRectF getPlugBounds() const;

        QRectF getKnobBounds() const;

        QRectF getSliderBounds(bool vertical) const;

        void paintPlug(QPainter *painter);

        void paintKnob(QPainter *painter);

        void paintSlider(QPainter *painter, bool vertical);
    };

}
