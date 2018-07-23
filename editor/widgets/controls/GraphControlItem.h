#pragma once

#include "ControlItem.h"

namespace AxiomModel {
    class GraphControl;
}

namespace AxiomGui {

    class GraphControlItem;

    class GraphControlTicks : public QGraphicsObject, public AxiomCommon::Hookable {
    public:
        GraphControlItem *item;

        explicit GraphControlTicks(GraphControlItem *item);

        void triggerUpdate();

        void triggerGeometryChange();

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    };

    class GraphControlZoom : public QGraphicsObject, public AxiomCommon::Hookable {
    public:
        AxiomModel::GraphControl *control;

        explicit GraphControlZoom(AxiomModel::GraphControl *control);

        void triggerUpdate();

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:
        bool isHovering = false;
        bool isDragging = false;

        void updateZoom(qreal mouseX);
    };

    class GraphControlItem : public ControlItem {
    public:
        AxiomModel::GraphControl *control;

        GraphControlItem(AxiomModel::GraphControl *control, NodeSurfaceCanvas *canvas);

        QRectF useBoundingRect() const override;

        void showHoverCursor(qreal pos);

        void moveHoverCursor(qreal pos);

        void hideHoverCursor();

    protected:
        bool showLabelInCenter() const override { return false; }

        QPainterPath controlPath() const override;

        void paintControl(QPainter *painter) override;

    private:
        void positionChildren();

        GraphControlTicks _ticks;
        GraphControlZoom _zoomer;
        QGraphicsLineItem _hoverCursor;
    };
}
