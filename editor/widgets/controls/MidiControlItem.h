#pragma once

#include "ControlItem.h"

#include "painters/PlugPainter.h"

namespace AxiomModel {
    class MidiControl;
}

namespace AxiomGui {

    class NodeItem;

    class MidiControlItem : public ControlItem {
    public:
        AxiomModel::MidiControl *control;

        MidiControlItem(AxiomModel::MidiControl *control, NodeSurfaceCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

    protected:

        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private:
        PlugPainter plugPainter;

    };

}
