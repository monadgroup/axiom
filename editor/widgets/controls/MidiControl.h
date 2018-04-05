#pragma once

#include "ControlItem.h"
#include "editor/model/connection/MidiConnectionSink.h"
#include "../CommonColors.h"

#include "painters/PlugPainter.h"

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

        QPainterPath controlPath() const override;

        QColor outlineNormalColor() const override { return CommonColors::midiNormal; }

        QColor outlineActiveColor() const override { return CommonColors::midiActive; }

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private:
        float m_hoverState = 0;
        bool isDragging = false;
        float beforeDragVal;
        QPointF mouseStartPoint;

        PlugPainter plugPainter;

    };

}
