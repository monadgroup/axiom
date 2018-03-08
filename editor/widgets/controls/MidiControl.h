#pragma once

#include "ControlItem.h"
#include "editor/model/connection/MidiConnectionSink.h"

namespace AxiomModel {
    class NodeMidiControl;
}

namespace AxiomGui {

    class NodeItem;

    class MidiControl : public ControlItem {
        Q_OBJECT
        Q_PROPERTY(float hoverState READ hoverState WRITE setHoverState)

    public:
        AxiomModel::NodeMidiControl *control;

        MidiControl(AxiomModel::NodeMidiControl *control, SchematicCanvas *canvas);

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

    private:
        float m_hoverState = 0;
        bool isDragging = false;
        float beforeDragVal;
        QPointF mouseStartPoint;

        QRectF getPlugBounds() const;

        QRectF getPianoBounds() const;

        void paintPlug(QPainter *painter);
    };

}
