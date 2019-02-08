#pragma once

#include "ControlItem.h"
#include "editor/model/objects/PortalControl.h"

namespace AxiomGui {

    class PortalControlItem : public ControlItem {
    public:
        AxiomModel::PortalControl *control;

        PortalControlItem(AxiomModel::PortalControl *control, NodeSurfaceCanvas *canvas);

        static QString iconNameFromType(AxiomModel::PortalControl::PortalType type,
                                        AxiomModel::ConnectionWire::WireType wireType);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:
        bool showLabelInCenter() const override { return true; }

        QRectF useBoundingRect() const override { return {}; }

        QPainterPath controlPath() const override;

        void paintControl(QPainter *painter) override {}

    private:
        QImage _image;

        static QString getImagePath(AxiomModel::PortalControl *control);
    };
}
