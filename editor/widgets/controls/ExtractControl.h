#pragma once

#include "ControlItem.h"
#include "editor/widgets/CommonColors.h"
#include "editor/model/connection/ExtractConnectionSink.h"

#include "painters/ExtractPainter.h"

namespace AxiomModel {
    class NodeExtractControl;
}

namespace AxiomGui {

    class NodeItem;

    class ExtractControl : public ControlItem {
    Q_OBJECT
        Q_PROPERTY(float hoverState READ hoverState WRITE setHoverState)

    public:
        AxiomModel::NodeExtractControl *control;

        ExtractControl(AxiomModel::NodeExtractControl *control, SchematicCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

        float hoverState() const { return m_hoverState; }

    public slots:

        void setHoverState(float newHoverState);

    signals:

        void mouseEnter();

        void mouseLeave();

    protected:
        QRectF useBoundingRect() const override;

        QPainterPath controlPath() const override;

        QColor outlineNormalColor() const override;

        QColor outlineActiveColor() const override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private:
        float m_hoverState = 0;
        ExtractPainter extractPainter;
    };

}
