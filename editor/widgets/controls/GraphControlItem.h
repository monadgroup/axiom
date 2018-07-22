#pragma once

#include "ControlItem.h"

namespace AxiomModel {
    class GraphControl;
}

namespace AxiomGui {

    class GraphControlZoom : public QGraphicsObject, public AxiomCommon::Hookable {
    public:
        AxiomModel::GraphControl *control;

        explicit GraphControlZoom(AxiomModel::GraphControl *control);

        void triggerUpdate();

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:
        /*void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;*/

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:
        bool isHovering;
    };

    class GraphControlItem : public ControlItem {
    public:
        AxiomModel::GraphControl *control;

        GraphControlItem(AxiomModel::GraphControl *control, NodeSurfaceCanvas *canvas);

    protected:
        bool showLabelInCenter() const override { return false; }

        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void paintControl(QPainter *painter) override;

    private:
        void positionChildren();

        void paintTicks(QPainter *painter, QRectF rect, float pixelsPerSecond);

        GraphControlZoom _zoomer;
    };
}
