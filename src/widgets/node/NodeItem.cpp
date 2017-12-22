#include "NodeItem.h"

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <iostream>

#include "../schematic/SchematicCanvas.h"
#include "src/model/Node.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *parent) : node(node) {
    //setFlag(QGraphicsItem::ItemIsMovable);
    //setFlag(QGraphicsItem::ItemContainsChildrenInShape);

    /*setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setAcceptHoverEvents(true);*/

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
    return {
            QPointF(node->pos().x() * SchematicCanvas::gridSize.width(), node->pos().y() * SchematicCanvas::gridSize.height()),
            QSizeF(node->size().height() * SchematicCanvas::gridSize.width(), node->size().height() * SchematicCanvas::gridSize.height())
    };
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
    update();
}

void NodeItem::remove() {
    parentItem()->scene()->removeItem(this);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (!boundingRect().contains(event->scenePos())) {
        event->ignore();
        return;
    }

    std::cout << "Mouse press" << std::endl;

    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        mouseStartPoint = event->screenPos();
        nodeStartPoint = node->pos();
    }

    event->accept();
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    std::cout << "Mouse move" << std::endl;
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;
    auto newNodePos = nodeStartPoint + QPoint(
        mouseDelta.x() / SchematicCanvas::gridSize.width(),
        mouseDelta.y() / SchematicCanvas::gridSize.height()
    );
    node->setPos(newNodePos);

    event->accept();
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    std::cout << "Mouse release" << std::endl;
    isDragging = false;

    event->accept();
}
