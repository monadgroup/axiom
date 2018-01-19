#include "NodeItem.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtCore/QTimer>

#include "editor/widgets/ItemResizer.h"
#include "../schematic/SchematicCanvas.h"
#include "editor/model/node/Node.h"
#include "editor/model/control/NodeControl.h"
#include "editor/model/control/NodeValueControl.h"
#include "../controls/BasicControl.h"
#include "NodePanel.h"

using namespace AxiomGui;
using namespace AxiomModel;

NodeItem::NodeItem(Node *node, SchematicCanvas *canvas) : node(node), canvas(canvas) {
    setAcceptHoverEvents(true);

    // create items for all controls that already exist
    for (const auto &item : node->surface.items()) {
        if (auto control = dynamic_cast<NodeControl *>(item.get())) {
            addControl(control);
        }
    }

    connect(node, &Node::nameChanged,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::posChanged,
            this, &NodeItem::setPos);
    connect(node, &Node::beforeSizeChanged,
            this, &NodeItem::triggerGeometryChange);
    connect(node, &Node::sizeChanged,
            this, &NodeItem::setSize);
    connect(node, &Node::selectedChanged,
            this, &NodeItem::setIsSelected);
    connect(node, &Node::deselected,
            this, &NodeItem::triggerUpdate);
    connect(node, &Node::removed,
            this, &NodeItem::remove);

    connect(&node->surface, &NodeSurface::hasSelectionChanged,
            this, &NodeItem::triggerUpdate);

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

        resizer->setParentItem(this);
    }

    // create panel
    nodePanel = new NodePanel(node);
    nodePanelProxy = canvas->addWidget(nodePanel);
    nodePanelProxy->setZValue(SchematicCanvas::panelZVal);

    // set initial state
    setPos(node->pos());
    setSize(node->size());
    setIsSelected(node->isSelected());
}

QRectF NodeItem::boundingRect() const {
    return {
            QPointF(0, 0),
            SchematicCanvas::nodeRealSize(node->size())
    };
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    painter->setPen(QPen(QColor::fromRgb(51, 51, 51), 1));
    if (node->isSelected()) {
        painter->setBrush(QBrush(QColor::fromRgb(27, 27, 27, 100)));
    } else {
        painter->setBrush(QBrush(QColor::fromRgb(17, 17, 17, 100)));
    }
    painter->drawRect(boundingRect());

    auto gridPen = QPen(QColor(27, 27, 27), 1);

    if (node->surface.hasSelection()) {
        auto nodeSurfaceSize = NodeSurface::schematicToNodeSurface(node->size());
        for (auto x = 0; x < nodeSurfaceSize.width(); x++) {
            for (auto y = 0; y < nodeSurfaceSize.height(); y++) {
                painter->drawPoint(SchematicCanvas::controlRealPos(QPoint(x, y)));
            }
        }
    }
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    node->surface.deselectAll();

    if (event->button() == Qt::LeftButton) {
        if (!node->isSelected()) node->select(!(event->modifiers() & Qt::ShiftModifier));

        isDragging = true;
        mouseStartPoint = event->screenPos();
        emit node->startedDragging();
    }

    event->accept();
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto mouseDelta = event->screenPos() - mouseStartPoint;
    emit node->draggedTo(QPoint(
            qRound((float) mouseDelta.x() / SchematicCanvas::nodeGridSize.width()),
            qRound((float) mouseDelta.y() / SchematicCanvas::nodeGridSize.height())
    ));

    event->accept();
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    emit node->finishedDragging();

    event->accept();
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    nodePanel->setNodeHover(true);
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    QTimer::singleShot(10, [this]() {
        nodePanel->setNodeHover(false);
    });
}

void NodeItem::setPos(QPoint newPos) {
    auto realPos = SchematicCanvas::nodeRealPos(newPos);
    updateNodePanelPos(realPos);
    QGraphicsItem::setPos(realPos.x(), realPos.y());
    emit resizerPosChanged(realPos);
}

void NodeItem::setSize(QSize newSize) {
    updateNodePanelPos(SchematicCanvas::nodeRealPos(node->pos()));
    nodePanelProxy->widget()->setFixedWidth(node->size().width() * SchematicCanvas::nodeGridSize.width());

    emit resizerSizeChanged(SchematicCanvas::nodeRealSize(newSize));
}

void NodeItem::addControl(NodeControl *control) {
    if (auto valueControl = dynamic_cast<NodeValueControl *>(control)) {
        auto c = new BasicControl(valueControl, canvas);
        c->setZValue(2);
        c->setParentItem(this);
    }
}

void NodeItem::setIsSelected(bool selected) {
    setZValue(selected ? SchematicCanvas::activeNodeZVal : SchematicCanvas::nodeZVal);
}

void NodeItem::remove() {
    scene()->removeItem(nodePanelProxy);
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

void NodeItem::triggerUpdate() {
    update();
}

void NodeItem::triggerGeometryChange() {
    prepareGeometryChange();
}

void NodeItem::updateNodePanelPos(QPointF realNodePos) {
    nodePanelProxy->setPos(realNodePos.x(),
                           realNodePos.y() + node->size().height() * SchematicCanvas::nodeGridSize.height() + 1);
}
