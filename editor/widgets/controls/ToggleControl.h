#pragma once

#include "ControlItem.h"

namespace AxiomModel {
    class NodeValueControl;
}

namespace AxiomGui {

    class ToggleControl : public ControlItem {
    Q_OBJECT
        Q_PROPERTY(float hoverState
                           READ
                                   hoverState
                           WRITE
                           setHoverState)

    public:
        AxiomModel::NodeValueControl *control;

        explicit ToggleControl(AxiomModel::NodeValueControl *control, SchematicCanvas *canvas);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPainterPath shape() const override;

        float hoverState() const { return m_hoverState; }

    public slots:

        void setHoverState(float newHoverState);

    signals:

        void mouseEnter();

        void mouseLeave();

    protected:

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:
        float m_hoverState = 0;

        QRectF getBounds() const;

    };

}
