#include "NodeItem.h"

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <iostream>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include "../schematic/SchematicCanvas.h"
#include "src/model/Node.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *parent) : node(node) {
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // set initial state
    setPos(node->pos());

    // connect to model
    connect(node, &Node::nameChanged,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::posChanged,
            this, &NodeItem::setPos);
    connect(node, &Node::sizeChanged,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::selectedChanged,
            this, &NodeItem::triggerUpdate);

    connect(node, &Node::removed,
            this, &NodeItem::remove);
}

QRectF NodeItem::boundingRect() const {
    return {
        QPointF(0, 0),
        QSizeF(node->size().height() * SchematicCanvas::gridSize.width(), node->size().height() * SchematicCanvas::gridSize.height())
    };
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setClipRect(option->exposedRect);

    painter->setPen(QPen(QColor::fromRgb(51, 51, 51), 1));
    painter->setBrush(QBrush(QColor::fromRgb(17, 17, 17)));
    painter->drawRect(boundingRect());

    painter->setPen(Qt::transparent);
    painter->setBrush(QBrush(Qt::white));
    painter->drawText(boundingRect(), node->name());
}

void NodeItem::triggerUpdate() {
    update();
}

void NodeItem::setPos(QPoint newPos) {
    QGraphicsObject::setPos(
        newPos.x() * SchematicCanvas::gridSize.width(),
        newPos.y() * SchematicCanvas::gridSize.height()
    );
    update();
}

void NodeItem::remove() {
    parentItem()->scene()->removeItem(this);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        mouseStartPoint = event->screenPos();
        nodeStartPoint = QPoint(
            node->pos().x() * SchematicCanvas::gridSize.width(),
            node->pos().y() * SchematicCanvas::gridSize.height()
        );
    }

    event->accept();
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;

    auto newNodePos = nodeStartPoint + mouseDelta;
    node->setPos(QPoint(
        qRound((float)newNodePos.x() / SchematicCanvas::gridSize.width()),
        qRound((float)newNodePos.y() / SchematicCanvas::gridSize.height())
    ));

    event->accept();
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;

    event->accept();
}
