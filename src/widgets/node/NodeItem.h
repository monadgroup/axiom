#pragma once
#include <QtWidgets/QGraphicsItem>

namespace AxiomGui {

    class NodeItem : public QGraphicsObject {
        Q_OBJECT

    public:
        explicit NodeItem(QGraphicsItem *parent = nullptr);

        QRectF boundingRect() const override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    };

}
