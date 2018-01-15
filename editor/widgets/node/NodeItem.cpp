#include "NodeItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsProxyWidget>

#include "editor/widgets/ItemResizer.h"
#include "NodeItemBackground.h"
#include "../schematic/SchematicCanvas.h"
#include "editor/model/node/Node.h"
#include "editor/model/control/NodeControl.h"
#include "editor/model/control/NodeValueControl.h"
#include "../controls/ControlItem.h"
#include "NodePanel.h"

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
    ItemResizer::Direction directions[] = {
            ItemResizer::TOP, ItemResizer::RIGHT, ItemResizer::BOTTOM, ItemResizer::LEFT,
            ItemResizer::TOP_RIGHT, ItemResizer::TOP_LEFT, ItemResizer::BOTTOM_RIGHT, ItemResizer::BOTTOM_LEFT
    };
    for (auto i = 0; i < 8; i++) {
        auto resizer = new ItemResizer(directions[i], SchematicCanvas::nodeGridSize);

        // ensure corners are on top of edges
        resizer->setZValue(i > 3 ? 2 : 1);

        connect(this, &NodeItem::resizerPosChanged,
                resizer, &ItemResizer::setPos);
        connect(this, &NodeItem::resizerSizeChanged,
                resizer, &ItemResizer::setSize);

        connect(resizer, &ItemResizer::startDrag,
                this, &NodeItem::resizerStartDrag);
        connect(resizer, &ItemResizer::changed,
                this, &NodeItem::resizerChanged);

        addToGroup(resizer);
    }

    // create panel
    nodePanel = new QGraphicsProxyWidget();
    nodePanel->setWidget(new NodePanel(node));
    addToGroup(nodePanel);

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
    nodePanel->setPos(0, node->size().height() * SchematicCanvas::nodeGridSize.height() + 2);
    nodePanel->widget()->setFixedWidth(node->size().width() * SchematicCanvas::nodeGridSize.width());

    emit resizerSizeChanged(SchematicCanvas::nodeRealSize(newSize));
}

void NodeItem::addControl(NodeControl *control) {
    auto c = new ControlItem(control, this);
    c->setZValue(2);
    addToGroup(c);
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
