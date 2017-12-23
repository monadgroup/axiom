#include "NodeItemContent.h"

#include <QtWidgets/QStyleOptionGraphicsItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "../schematic/SchematicCanvas.h"
#include "src/model/Node.h"
#include "src/model/Schematic.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItemContent::NodeItemContent(Node *node) : node(node) {
    connect(node, &Node::nameChanged,
            this, &NodeItemContent::triggerUpdate);
    connect(node, &Node::beforeSizeChanged,
            this, &NodeItemContent::triggerGeometryChange);
    connect(node, &Node::selected,
            this, &NodeItemContent::triggerUpdate);
    connect(node, &Node::deselected,
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
    if (node->isSelected()) {
        painter->setBrush(QBrush(QColor::fromRgb(25, 25, 25)));
    } else {
        painter->setBrush(QBrush(QColor::fromRgb(17, 17, 17)));
    }
    painter->drawRect(boundingRect());

    painter->setPen(Qt::transparent);
    painter->setBrush(QBrush(Qt::white));
    painter->drawText(boundingRect(), node->name());
}

void NodeItemContent::triggerUpdate() {
    update();
}

void NodeItemContent::triggerGeometryChange() {
    prepareGeometryChange();
}

void NodeItemContent::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (!node->isSelected()) node->select(true);

        isDragging = true;
        mouseStartPoint = event->screenPos();
        emit node->startedDragging();
    }

    event->accept();
}

void NodeItemContent::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;
    emit node->draggedTo(QPoint(
            qRound((float) mouseDelta.x() / SchematicCanvas::gridSize.width()),
            qRound((float) mouseDelta.y() / SchematicCanvas::gridSize.height())
    ));

    event->accept();
}

void NodeItemContent::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    emit node->finishedDragging();

    event->accept();
}
