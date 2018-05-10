#pragma once

#include "ControlItem.h"

#include "painters/KnobPainter.h"
#include "painters/PlugPainter.h"
#include "painters/SliderPainter.h"
#include "painters/TogglePainter.h"

namespace AxiomModel {
    class NumControl;
}

namespace AxiomGui {

    class NodeItem;

    class NumControlItem : public ControlItem {
    public:
        AxiomModel::NumControl *control;

        NumControlItem(AxiomModel::NumControl *control, NodeSurfaceCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

    protected:
        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void wheelEvent(QGraphicsSceneWheelEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private slots:

        void setValue(QString value);

    private:
        bool isDragging = false;
        MaximRuntime::NumValue beforeDragVal;
        QPointF mouseStartPoint;

        KnobPainter knobPainter;
        PlugPainter plugPainter;
        SliderPainter sliderPainter;
        TogglePainter togglePainter;

        QString valueAsString(MaximRuntime::NumValue num);

        MaximRuntime::NumValue stringAsValue(const QString &str, MaximRuntime::NumValue oldNum);

        MaximRuntime::NumValue getCVal() const;

        void setCVal(MaximRuntime::NumValue v) const;
    };

}
