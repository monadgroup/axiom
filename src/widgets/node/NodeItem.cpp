#include "NodeItem.h"

#include <QtGui/QPen>
#include <QtGui/QPainter>

#include "../schematic/SchematicCanvas.h"
#include "src/model/Node.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *parent) : node(node) {
    // connect to model
    connect(node, &Node::nameChanged,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::posChanged,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::sizeChanged,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::selectedChanged,
            this, &NodeItem::triggerUpdate);

    connect(node, &Node::removed,
            this, &NodeItem::remove);
}

QRectF NodeItem::boundingRect() const {
    return {150, 200, 100, 100};
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(QPen(QColor::fromRgb(51, 51, 51), 1));
    painter->setBrush(QBrush(QColor::fromRgb(17, 17, 17)));
    painter->drawRect(boundingRect());

    painter->setPen(Qt::transparent);
    painter->setBrush(QBrush(Qt::white));
    painter->drawText(boundingRect(), node->name());
}

void NodeItem::triggerUpdate() {
    update(boundingRect());
}

void NodeItem::remove() {
    parentItem()->scene()->removeItem(this);
}
