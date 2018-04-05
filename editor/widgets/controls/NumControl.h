#pragma once

#include "ControlItem.h"
#include "editor/widgets/CommonColors.h"
#include "editor/model/connection/NumConnectionSink.h"

#include "painters/KnobPainter.h"
#include "painters/PlugPainter.h"
#include "painters/SliderPainter.h"
#include "painters/TogglePainter.h"

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

        QColor outlineNormalColor() const override { return CommonColors::numNormal; }

        QColor outlineActiveColor() const override { return CommonColors::numActive; }

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
