#pragma once

#include "ControlItem.h"

namespace AxiomModel {
    class NodeValueControl;
}

namespace AxiomGui {

    class NodeItem;

    class BasicControl : public ControlItem {
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

        explicit BasicControl(AxiomModel::NodeValueControl *control, SchematicCanvas *canvas);

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

        void wheelEvent(QGraphicsSceneWheelEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private slots:

        void setValue(QString value);

    private:
        float m_hoverState = 0;
        bool isDragging = false;
        float beforeDragVal = 0;
        QPointF mouseStartPoint;

        QRectF getPlugBounds() const;

        QRectF getKnobBounds() const;

        QRectF getSliderBounds(bool vertical) const;

        void paintPlug(QPainter *painter);

        void paintKnob(QPainter *painter);

        void paintSlider(QPainter *painter, bool vertical);
    };

}
