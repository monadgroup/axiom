#pragma once

#include "ControlItem.h"

namespace AxiomModel {
    class PortalControl;
}

namespace AxiomGui {

    class PortalControlItem : public ControlItem {
    public:
        AxiomModel::PortalControl *control;

        PortalControlItem(AxiomModel::PortalControl *control, NodeSurfaceCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

    protected:

        QRectF useBoundingRect() const override { return {}; }

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        QPainterPath controlPath() const override { return shape(); }

    private:

        QImage _image;

        static QString getImagePath(AxiomModel::PortalControl *control);
    };

}
