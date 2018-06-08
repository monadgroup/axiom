#pragma once

#include <QtCore/QTimer>

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

        static QString formatNumber(float val, MaximCommon::FormType form);

        QPainterPath shape() const override;

    protected:
        bool showLabelInCenter() const override;

        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void wheelEvent(QGraphicsSceneWheelEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

        void paintControl(QPainter *painter) override;

        QString getLabelText() const override;

    private slots:

        void setStringValue(QString value);

        void setValue(MaximRuntime::NumValue value);

        void controlValueChanged();

        void showValueExpired();

    private:
        bool isDragging = false;
        bool isShowingValue = false;
        QTimer showValueTimer;
        MaximRuntime::NumValue beforeDragVal;
        QPointF mouseStartPoint;

        KnobPainter knobPainter;
        PlugPainter plugPainter;
        SliderPainter sliderPainter;
        TogglePainter togglePainter;

        QString valueAsString(MaximRuntime::NumValue num);

        MaximRuntime::NumValue stringAsValue(const QString &str, MaximRuntime::NumValue oldNum);

        static MaximRuntime::NumValue clampVal(const MaximRuntime::NumValue &val);

        MaximRuntime::NumValue getCVal() const;

        void setCVal(MaximRuntime::NumValue v) const;
    };

}
