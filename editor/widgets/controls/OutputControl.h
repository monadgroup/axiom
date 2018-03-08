#pragma once

#include "ControlItem.h"
#include "../CommonColors.h"

namespace AxiomModel {
    class NodeOutputControl;
}

namespace AxiomGui {

    class OutputControl : public ControlItem {
    public:
        AxiomModel::NodeOutputControl *control;

        OutputControl(AxiomModel::NodeOutputControl *control, SchematicCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

    protected:

        QRectF useBoundingRect() const override { return {}; }

        QPainterPath controlPath() const override { return shape(); }

        QColor outlineNormalColor() const override { return CommonColors::numWireNormal; }

        QColor outlineActiveColor() const override { return CommonColors::numWireActive; }

    private:

        QImage _image;
    };

}
