#include "NodeItemBackground.h"

#include <QtWidgets/QStyleOptionGraphicsItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "../schematic/SchematicCanvas.h"
#include "editor/model/node/Node.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItemBackground::NodeItemBackground(Node *node) : node(node) {
    connect(node, &Node::nameChanged,
            this, &NodeItemBackground::triggerUpdate);
    connect(node, &Node::beforeSizeChanged,
            this, &NodeItemBackground::triggerGeometryChange);
    connect(node, &Node::selected,
            this, &NodeItemBackground::triggerUpdate);
    connect(node, &Node::deselected,
            this, &NodeItemBackground::triggerUpdate);
    connect(&node->surface, &NodeSurface::hasSelectionChanged,
            this, &NodeItemBackground::triggerUpdate);
}

QRectF NodeItemBackground::boundingRect() const {
    return {
            QPointF(0, 0),
            SchematicCanvas::nodeRealSize(node->size())
    };
}

void NodeItemBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setClipRect(option->exposedRect);

    painter->setPen(QPen(QColor::fromRgb(51, 51, 51), 1));
    if (node->isSelected()) {
        painter->setBrush(QBrush(QColor::fromRgb(27, 27, 27)));
    } else {
        painter->setBrush(QBrush(QColor::fromRgb(17, 17, 17)));
    }
    painter->drawRect(boundingRect());

    auto gridPen = QPen(QColor(51, 51, 51), 1);

    if (node->surface.hasSelection()) {
        auto nodeSurfaceSize = NodeSurface::schematicToNodeSurface(node->size());
        for (auto x = 0; x < nodeSurfaceSize.width(); x++) {
            for (auto y = 0; y < nodeSurfaceSize.height(); y++) {
                painter->drawPoint(SchematicCanvas::controlRealPos(QPoint(x, y)));
            }
        }
    }
}

void NodeItemBackground::triggerUpdate() {
    update();
}

void NodeItemBackground::triggerGeometryChange() {
    prepareGeometryChange();
}

void NodeItemBackground::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    node->surface.deselectAll();

    if (event->button() == Qt::LeftButton) {
        if (!node->isSelected()) node->select(!(event->modifiers() & Qt::ShiftModifier));

        isDragging = true;
        mouseStartPoint = event->screenPos();
        emit node->startedDragging();
    }

    event->accept();
}

void NodeItemBackground::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;
    emit node->draggedTo(QPoint(
            qRound((float) mouseDelta.x() / SchematicCanvas::nodeGridSize.width()),
            qRound((float) mouseDelta.y() / SchematicCanvas::nodeGridSize.height())
    ));

    event->accept();
}

void NodeItemBackground::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    emit node->finishedDragging();

    event->accept();
}
