#pragma once

#include "ControlItem.h"
#include "editor/model/connection/NumConnectionSink.h"

namespace AxiomModel {
    class NodeNumControl;
}

namespace AxiomGui {

    class NodeItem;

    class NumControl : public ControlItem {
    Q_OBJECT
        Q_PROPERTY(float hoverState
                           READ
                                   hoverState
                           WRITE
                           setHoverState)

    public:
        AxiomModel::NodeNumControl *control;

        NumControl(AxiomModel::NodeNumControl *control, SchematicCanvas *canvas);

        QRectF aspectBoundingRect() const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

        float hoverState() const { return m_hoverState; }

    public slots:

        void setHoverState(float newHoverState);

    signals:

        void mouseEnter();

        void mouseLeave();

    protected:
        QRectF useBoundingRect() const override;

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
        AxiomModel::NumValue beforeDragVal;
        QPointF mouseStartPoint;

        QRectF getPlugBounds() const;

        QRectF getKnobBounds() const;

        QRectF getSliderBounds(bool vertical) const;

        QRectF getToggleBounds() const;

        void paintPlug(QPainter *painter);

        void paintKnob(QPainter *painter);

        void paintSlider(QPainter *painter, bool vertical);

        void paintToggle(QPainter *painter);

        QString valueAsString(AxiomModel::NumValue num);

        AxiomModel::NumValue stringAsValue(const QString &str, AxiomModel::NumValue oldNum);

        AxiomModel::NumValue getCVal() const;

        void setCVal(AxiomModel::NumValue v) const;
    };

}
