#pragma once
#include <QtWidgets/QGraphicsItem>

namespace AxiomModel {
    class Node;
}

namespace AxiomGui {

    class SchematicCanvas;

    class NodeItem : public QGraphicsObject {
        Q_OBJECT

    public:
        AxiomModel::Node *node;

        explicit NodeItem(AxiomModel::Node *node, SchematicCanvas *parent);

        QRectF boundingRect() const override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    public slots:
        void triggerUpdate();

        void remove();
    };

}
