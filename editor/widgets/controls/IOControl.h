#pragma once

#include "ControlItem.h"
#include "../CommonColors.h"

namespace AxiomModel {
    class NodeIOControl;
}

namespace AxiomGui {

    class IOControl : public ControlItem {
    public:
        AxiomModel::NodeIOControl *control;

        IOControl(AxiomModel::NodeIOControl *control, SchematicCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

    protected:

        QRectF useBoundingRect() const override { return {}; }

        QPainterPath controlPath() const override { return shape(); }

        QColor outlineNormalColor() const override { return CommonColors::numWireNormal; }

        QColor outlineActiveColor() const override { return CommonColors::numWireActive; }

    private:

        QImage _image;

        static QString getImagePath(AxiomModel::NodeIOControl *control);
    };

}
