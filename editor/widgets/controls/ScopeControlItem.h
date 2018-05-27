#pragma once

#include "ControlItem.h"

namespace AxiomModel {
    class ScopeControl;
}

namespace AxiomGui {

    class ScopeControlItem : public ControlItem {
    public:
        AxiomModel::ScopeControl *control;

        ScopeControlItem(AxiomModel::ScopeControl *control, NodeSurfaceCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

    protected:
        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    };

}
