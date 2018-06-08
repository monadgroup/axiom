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

        bool showLabelInCenter() const override { return true; }

        QRectF useBoundingRect() const override { return {}; }

        QPainterPath controlPath() const override { return shape(); }

        void paintControl(QPainter *painter) override {}

    private:

        QImage _image;

        static QString getImagePath(AxiomModel::PortalControl *control);
    };

}
