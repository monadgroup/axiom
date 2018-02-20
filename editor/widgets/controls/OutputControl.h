#pragma once

#include "ControlItem.h"

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

        virtual QRectF useBoundingRect() const { return {}; }

    private:

        QImage _image;
    };

}
