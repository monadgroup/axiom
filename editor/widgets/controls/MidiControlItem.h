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

    protected:
        bool showLabelInCenter() const override { return true; }

        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

        void paintControl(QPainter *painter) override;

    private:
        PlugPainter plugPainter;
    };
}
