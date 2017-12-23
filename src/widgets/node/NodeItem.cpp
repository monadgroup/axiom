#include "NodeItem.h"

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include "NodeResizer.h"
#include "NodeItemContent.h"
#include "../schematic/SchematicCanvas.h"
#include "src/model/Node.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *parent) : node(node) {
    setHandlesChildEvents(false);

    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    connect(node, &Node::posChanged,
            this, &NodeItem::setPos);
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
    for (const auto dir : directions) {
        auto resizer = new NodeResizer(dir);
        resizer->setZValue(1);
        connect(this, &NodeItem::resizerPosChanged,
                resizer, &NodeResizer::setPos);
        connect(this, &NodeItem::resizerSizeChanged,
                resizer, &NodeResizer::setSize);

        connect(resizer, &NodeResizer::moved,
                this, &NodeItem::resizerSetPos);
        connect(resizer, &NodeResizer::resized,
                this, &NodeItem::resizerSetSize);

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
    update();
}

void NodeItem::remove() {
    parentItem()->scene()->removeItem(this);
}

void NodeItem::resizerSetPos(QPointF newPos) {
    node->setPos(QPoint(
        qRound(newPos.x() / SchematicCanvas::gridSize.width()),
        qRound(newPos.y() / SchematicCanvas::gridSize.height())
    ));
}

void NodeItem::resizerSetSize(QSizeF newSize) {
    node->setSize(QSize(
        qRound(newSize.width() / SchematicCanvas::gridSize.width()),
        qRound(newSize.height() / SchematicCanvas::gridSize.height())
    ));
}
