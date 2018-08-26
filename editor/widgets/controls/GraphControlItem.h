#pragma once

#include <QtWidgets/QGraphicsPathItem>
#include <memory>

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

    class GraphControlPointKnob : public QGraphicsObject {
    public:
        GraphControlItem *item;
        uint8_t index;
        qreal scale;
        qreal minY;
        qreal maxY;
        float minSeconds;
        float maxSeconds;
        double snapSeconds;

        GraphControlPointKnob(GraphControlItem *item, uint8_t index);

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
        QPointF dragStartMousePos;
        float dragStartYVal;
        float dragStartTime;
    };

    class GraphControlTensionKnob : public QGraphicsObject {
    public:
        AxiomModel::GraphControl *control;
        uint8_t index;
        qreal movementRange;

        GraphControlTensionKnob(AxiomModel::GraphControl *control, uint8_t index);

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
        qreal dragStartMouseY;
        qreal dragStartTension;
    };

    class GraphControlArea : public QGraphicsObject, public AxiomCommon::Hookable {
    public:
        GraphControlItem *item;
        QRectF clipBounds;
        QRectF drawBounds;

        explicit GraphControlArea(GraphControlItem *item);

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        void updateBounds(QRectF newClipBounds, QRectF newDrawBounds);

        void updateCurves();

    private:
        std::vector<std::unique_ptr<QGraphicsPathItem>> _curves;
        std::vector<std::unique_ptr<GraphControlTensionKnob>> _tensionKnobs;
        std::vector<std::unique_ptr<GraphControlPointKnob>> _pointKnobs;
    };

    class GraphControlItem : public ControlItem {
    public:
        AxiomModel::GraphControl *control;

        GraphControlItem(AxiomModel::GraphControl *control, NodeSurfaceCanvas *canvas);

        QRectF useBoundingRect() const override;

        void setShowSnapMarks(bool value);

    protected:
        bool showLabelInCenter() const override { return false; }

        QPainterPath controlPath() const override;

        void paintControl(QPainter *painter) override;

    private:
        void positionChildren();

        void stateChange();

        GraphControlTicks _ticks;
        GraphControlZoom _zoomer;
        GraphControlArea _area;
        bool _showSnapMarks = false;
    };
}
