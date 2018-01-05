#include "NodeItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "NodeResizer.h"
#include "NodeItemContent.h"
#include "../schematic/SchematicCanvas.h"
#include "src/model/node/Node.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *parent) : node(node) {
    setHandlesChildEvents(false);

    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    connect(node, &Node::posChanged,
            this, &NodeItem::setPos);
    connect(node, &Node::beforeSizeChanged,
            this, &NodeItem::triggerGeometryChange);
    connect(node, &Node::sizeChanged,
            this, &NodeItem::setSize);
    connect(node, &Node::removed,
            this, &NodeItem::remove);

    // create main item
    auto content = new NodeItemContent(node);
    content->setZValue(0);
    addToGroup(content);

    // create resize items
    NodeResizer::Direction directions[] = {
            NodeResizer::TOP, NodeResizer::RIGHT, NodeResizer::BOTTOM, NodeResizer::LEFT,
            NodeResizer::TOP_RIGHT, NodeResizer::TOP_LEFT, NodeResizer::BOTTOM_RIGHT, NodeResizer::BOTTOM_LEFT
    };
    for (auto i = 0; i < 8; i++) {
        auto resizer = new NodeResizer(directions[i], SchematicCanvas::gridSize);

        // ensure corners are on top of edges
        resizer->setZValue(i > 3 ? 2 : 1);

        connect(this, &NodeItem::resizerPosChanged,
                resizer, &NodeResizer::setPos);
        connect(this, &NodeItem::resizerSizeChanged,
                resizer, &NodeResizer::setSize);

        connect(resizer, &NodeResizer::startDrag,
                this, &NodeItem::resizerStartDrag);
        connect(resizer, &NodeResizer::changed,
                this, &NodeItem::resizerChanged);

        addToGroup(resizer);
    }

    // set initial state
    setPos(node->pos());
    setSize(node->size());
}

void NodeItem::setPos(QPoint newPos) {
    auto realPos = SchematicCanvas::nodeRealPos(newPos);
    QGraphicsItem::setPos(realPos.x(), realPos.y());
    emit resizerPosChanged(realPos);
}

void NodeItem::setSize(QSize newSize) {
    emit resizerSizeChanged(SchematicCanvas::nodeRealSize(newSize));
}

void NodeItem::remove() {
    scene()->removeItem(this);
}

void NodeItem::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    node->setCorners(QPoint(
            qRound(topLeft.x() / SchematicCanvas::gridSize.width()),
            qRound(topLeft.y() / SchematicCanvas::gridSize.height())
    ), QPoint(
            qRound(bottomRight.x() / SchematicCanvas::gridSize.width()),
            qRound(bottomRight.y() / SchematicCanvas::gridSize.height())
    ));
}

void NodeItem::resizerStartDrag() {
    node->select(true);
}

void NodeItem::triggerGeometryChange() {
    prepareGeometryChange();
}
