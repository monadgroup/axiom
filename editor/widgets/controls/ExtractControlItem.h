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

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

    protected:
        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private:
        ExtractPainter extractPainter;
    };

}
