#include "NodeItemContent.h"

#include <QtWidgets/QStyleOptionGraphicsItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "../schematic/SchematicCanvas.h"
#include "src/model/Node.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItemContent::NodeItemContent(Node *node) : node(node) {
    connect(node, &Node::nameChanged,
            this, &NodeItemContent::triggerUpdate);
    connect(node, &Node::sizeChanged,
            this, &NodeItemContent::triggerUpdate);
    connect(node, &Node::selectedChanged,
            this, &NodeItemContent::triggerUpdate);
}

QRectF NodeItemContent::boundingRect() const {
    return {
        QPointF(0, 0),
        SchematicCanvas::nodeRealSize(node->size())
    };
}

void NodeItemContent::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setClipRect(option->exposedRect);

    painter->setPen(QPen(QColor::fromRgb(51, 51, 51), 1));
    painter->setBrush(QBrush(QColor::fromRgb(17, 17, 17)));
    painter->drawRect(boundingRect());

    painter->setPen(Qt::transparent);
    painter->setBrush(QBrush(Qt::white));
    painter->drawText(boundingRect(), node->name());
}

void NodeItemContent::triggerUpdate() {
    update();
}

void NodeItemContent::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        mouseStartPoint = event->screenPos();
        nodeStartPoint = SchematicCanvas::nodeRealPos(node->pos());
    }

    event->accept();
}

void NodeItemContent::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;

    auto newNodePos = nodeStartPoint + mouseDelta;
    node->setPos(QPoint(
            qRound((float)newNodePos.x() / SchematicCanvas::gridSize.width()),
            qRound((float)newNodePos.y() / SchematicCanvas::gridSize.height())
    ));

    event->accept();
}

void NodeItemContent::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;

    event->accept();
}
