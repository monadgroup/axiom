#include "NodeItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "NodeResizer.h"
#include "NodeItemBackground.h"
#include "../schematic/SchematicCanvas.h"
#include "src/model/node/Node.h"
#include "src/model/control/NodeControl.h"
#include "src/model/control/NodeValueControl.h"
#include "../controls/KnobControl.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *parent) : node(node) {
    setHandlesChildEvents(false);

    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // create main item
    auto background = new NodeItemBackground(node);
    background->setZValue(0);
    addToGroup(background);

    // create items for all controls that already exist
    for (const auto &item : node->surface.items()) {
        if (auto control = dynamic_cast<NodeControl *>(item.get())) {
            addControl(control);
        }
    }

    connect(node, &Node::posChanged,
            this, &NodeItem::setPos);
    connect(node, &Node::beforeSizeChanged,
            this, &NodeItem::triggerGeometryChange);
    connect(node, &Node::sizeChanged,
            this, &NodeItem::setSize);
    connect(node, &Node::removed,
            this, &NodeItem::remove);

    connect(&node->surface, &NodeSurface::itemAdded,
            [this](AxiomModel::GridItem *item) {
                if (auto control = dynamic_cast<NodeControl *>(item)) {
                    addControl(control);
                }
            });

    // create resize items
    NodeResizer::Direction directions[] = {
            NodeResizer::TOP, NodeResizer::RIGHT, NodeResizer::BOTTOM, NodeResizer::LEFT,
            NodeResizer::TOP_RIGHT, NodeResizer::TOP_LEFT, NodeResizer::BOTTOM_RIGHT, NodeResizer::BOTTOM_LEFT
    };
    for (auto i = 0; i < 8; i++) {
        auto resizer = new NodeResizer(directions[i], SchematicCanvas::nodeGridSize);

        // ensure corners are on top of edges
        resizer->setZValue(i > 3 ? 3 : 2);

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

void NodeItem::addControl(NodeControl *control) {
    if (auto valueControl = dynamic_cast<NodeValueControl *>(control)) {
        auto c = new KnobControl(valueControl, this);
        c->setZValue(1);
        addToGroup(c);
    }
}

void NodeItem::remove() {
    scene()->removeItem(this);
}

void NodeItem::resizerChanged(QPointF topLeft, QPointF bottomRight) {
    node->setCorners(QPoint(
            qRound(topLeft.x() / SchematicCanvas::nodeGridSize.width()),
            qRound(topLeft.y() / SchematicCanvas::nodeGridSize.height())
    ), QPoint(
            qRound(bottomRight.x() / SchematicCanvas::nodeGridSize.width()),
            qRound(bottomRight.y() / SchematicCanvas::nodeGridSize.height())
    ));
}

void NodeItem::resizerStartDrag() {
    node->select(true);
}

void NodeItem::triggerGeometryChange() {
    prepareGeometryChange();
}
