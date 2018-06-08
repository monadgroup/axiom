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

        QPainterPath shape() const override;

    protected:
        bool showLabelInCenter() const override { return false; }

        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

        void paintControl(QPainter *painter) override;
    };

}
