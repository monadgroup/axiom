#pragma once

#include "ControlItem.h"
#include "editor/widgets/CommonColors.h"

#include "painters/ExtractPainter.h"

namespace AxiomModel {
    class ExtractControl;
}

namespace AxiomGui {

    class NodeItem;

    class ExtractControlItem : public ControlItem {
    Q_OBJECT
    public:
        AxiomModel::ExtractControl *control;

        ExtractControlItem(AxiomModel::ExtractControl *control, NodeSurfaceCanvas *canvas);

        QPainterPath shape() const override;

    protected:
        bool showLabelInCenter() const override { return true; }

        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

        void paintControl(QPainter *painter) override;

    private:
        ExtractPainter extractPainter;
    };

}
