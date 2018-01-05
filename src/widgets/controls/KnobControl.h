#pragma once

#include <QtWidgets/QGraphicsObject>

namespace AxiomModel {
    class NodeValueControl;
}

namespace AxiomGui {

    class NodeItem;

    class KnobControl : public QGraphicsObject {
    Q_OBJECT
    public:
        AxiomModel::NodeValueControl *control;
        NodeItem *parent;

        KnobControl(AxiomModel::NodeValueControl *control, NodeItem *parent);

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    private slots:

        void setPos(QPoint newPos);

        void setSize(QSize newSize);

        void remove();

        void triggerGeometryChange();
    };

}
