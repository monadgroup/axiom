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

        static bool unformatString(const QString &str, float *valOut, AxiomModel::FormType *formOut);

    protected:
        bool showLabelInCenter() const override;

        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void wheelEvent(QGraphicsSceneWheelEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

        void paintControl(QPainter *painter) override;

        QString getLabelText() const override;

    private slots:

        void setStringValue(QString value);

        void setValue(AxiomModel::NumValue value);

        void controlValueChanged();

        void showValueExpired();

    private:
        bool isDragging = false;
        bool isShowingValue = false;
        bool displayNameOverride = false;
        bool canReplaceHistoryOnScroll = false;
        QImage _plugImage;
        QTimer showValueTimer;
        AxiomModel::NumValue beforeDragVal;
        QPointF mouseStartPoint;

        KnobPainter knobPainter;
        PlugPainter plugPainter;
        SliderPainter sliderPainter;
        TogglePainter togglePainter;

        AxiomModel::NumValue clampValue(AxiomModel::NumValue value);

        AxiomModel::NumValue getNormalizedValue();

        void setNormalizedValue(AxiomModel::NumValue val);

        AxiomModel::NumValue stringAsValue(const QString &str, AxiomModel::NumValue oldNum);

        void editNumRange(bool selectStep, QPointF pos);
    };
}
