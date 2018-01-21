#include "SchematicCanvas.h"

#include <cmath>
#include <cassert>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsPathItem>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QWidgetAction>

#include "editor/AxiomApplication.h"
#include "editor/model/node/CustomNode.h"
#include "editor/model/control/NodeValueControl.h"
#include "../node/NodeItem.h"
#include "../connection/WireItem.h"
#include "../IConnectable.h"

using namespace AxiomGui;
using namespace AxiomModel;

QSize SchematicCanvas::nodeGridSize = QSize(50, 50);

QSize SchematicCanvas::controlGridSize = QSize(25, 25);

int SchematicCanvas::wireZVal = 0;
int SchematicCanvas::activeWireZVal = 1;
int SchematicCanvas::nodeZVal = 2;
int SchematicCanvas::activeNodeZVal = 3;
int SchematicCanvas::panelZVal = 4;
int SchematicCanvas::selectionZVal = 5;

SchematicCanvas::SchematicCanvas(SchematicPanel *panel, Schematic *schematic) : panel(panel), schematic(schematic) {
    // build selection
    auto selectionPen = QPen(QColor::fromRgb(52, 152, 219));
    auto selectionBrush = QBrush(QColor::fromRgb(52, 152, 219, 50));

    selectionPath = addPath(QPainterPath(), selectionPen, selectionBrush);
    selectionPath->setVisible(false);
    selectionPath->setZValue(selectionZVal);

    // create items for all nodes & wires that already exist
    for (const auto &item : schematic->items()) {
        if (auto node = dynamic_cast<Node *>(item.get())) {
            addNode(node);
        }
    }

    for (const auto &wire : schematic->wires()) {
        addWire(wire.get());
    }

    // connect to model
    connect(schematic, &Schematic::itemAdded,
            [this](AxiomModel::GridItem *item) {
                if (auto node = dynamic_cast<Node *>(item)) {
                    addNode(node);
                }
            });
    connect(schematic, &Schematic::wireAdded,
            this, &SchematicCanvas::addWire);

    newNode(QPointF(0, 0));
}

QPoint SchematicCanvas::nodeRealPos(const QPoint &p) {
    return {
            p.x() * SchematicCanvas::nodeGridSize.width(),
            p.y() * SchematicCanvas::nodeGridSize.height()
    };
}

QSize SchematicCanvas::nodeRealSize(const QSize &s) {
    return {
            s.width() * SchematicCanvas::nodeGridSize.width(),
            s.height() * SchematicCanvas::nodeGridSize.height()
    };
}

QPoint SchematicCanvas::controlRealPos(const QPoint &p) {
    return {
            p.x() * SchematicCanvas::controlGridSize.width(),
            p.y() * SchematicCanvas::controlGridSize.height()
    };
}

QPointF SchematicCanvas::controlRealPos(const QPointF &p) {
    return {
            p.x() * SchematicCanvas::controlGridSize.width(),
            p.y() * SchematicCanvas::controlGridSize.height()
    };
}

QSize SchematicCanvas::controlRealSize(const QSize &s) {
    return {
            s.width() * SchematicCanvas::controlGridSize.width(),
            s.height() * SchematicCanvas::controlGridSize.height()
    };
}

void SchematicCanvas::startConnecting(IConnectable *control) {
    if (isConnecting) return;

    isConnecting = true;
    connectionSink = std::make_unique<ConnectionSink>(control->sink()->type);
    connectionSink->setPos(control->sink()->pos(), control->sink()->subPos());
    connectionWire = schematic->connectSinks(control->sink(), connectionSink.get());
    assert(connectionWire != nullptr);

    connect(connectionWire, &ConnectionWire::removed,
            [this]() { isConnecting = false; });
}

void SchematicCanvas::updateConnecting(QPointF mousePos) {
    if (!isConnecting) return;

    auto currentItem = itemAt(mousePos, QTransform());
    if (auto connectable = dynamic_cast<IConnectable *>(currentItem)) {
        connectionSink->setPos(connectable->sink()->pos(), connectable->sink()->subPos());
    } else {
        connectionSink->setPos(
                QPoint(
                        (int) (mousePos.x() / SchematicCanvas::nodeGridSize.width()),
                        (int) (mousePos.y() / SchematicCanvas::nodeGridSize.height())
                ),
                QPointF(
                        mousePos.x() / SchematicCanvas::controlGridSize.width(),
                        mousePos.y() / SchematicCanvas::controlGridSize.height()
                )
        );
    }
}

void SchematicCanvas::endConnecting(QPointF mousePos) {
    if (!isConnecting) return;

    auto currentItem = itemAt(mousePos, QTransform());

    if (auto connectable = dynamic_cast<IConnectable *>(currentItem)) {
        isConnecting = false;
        schematic->connectSinks(connectionWire->sinkA, connectable->sink());
    } else {
        // todo: create JunctionNode if not currentItem is null
    }

    connectionWire->remove();
    connectionSink.reset();
}

void SchematicCanvas::cancelConnecting() {
    if (!isConnecting) return;

    connectionWire->remove();
}

void SchematicCanvas::addNode(AxiomModel::Node *node) {
    auto item = new NodeItem(node, this);
    item->setZValue(nodeZVal);
    addItem(item);
}

void SchematicCanvas::newNode(QPointF scenePos) {
    auto defaultSize = QSize(5, 5);
    auto targetPos = QPoint(
            qRound((float) scenePos.x() / SchematicCanvas::nodeGridSize.width()) - defaultSize.width() / 2,
            qRound((float) scenePos.y() / SchematicCanvas::nodeGridSize.height()) - defaultSize.height() / 2
    );

    auto newNode = std::make_unique<CustomNode>(schematic, tr("New Node"), targetPos, defaultSize);

    auto newControl = std::make_unique<NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(0, 0), QSize(2, 2)
    );
    newControl->setValue(0.2);
    newNode->surface.addItem(std::move(newControl));

    auto newControl2 = std::make_unique<NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(2, 0), QSize(2, 2)
    );
    newControl2->setValue(0.4);
    newNode->surface.addItem(std::move(newControl2));

    auto newControl3 = std::make_unique<NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(0, 2), QSize(2, 6)
    );
    newNode->surface.addItem(std::move(newControl3));

    auto newControl4 = std::make_unique<NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::BASIC,
            NodeValueControl::Channel::BOTH,
            QPoint(2, 2), QSize(6, 2)
    );
    newNode->surface.addItem(std::move(newControl4));

    auto newControl5 = std::make_unique<NodeValueControl>(
            newNode.get(),
            NodeValueControl::Type::TOGGLE,
            NodeValueControl::Channel::BOTH,
            QPoint(4, 4), QSize(2, 2)
    );
    newNode->surface.addItem(std::move(newControl5));

    schematic->addItem(std::move(newNode));
}

void SchematicCanvas::addWire(AxiomModel::ConnectionWire *wire) {
    auto item = new WireItem(wire);
    item->setZValue(wireZVal);
    addItem(item);
}

void SchematicCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
    drawGrid(painter, rect, nodeGridSize, QColor::fromRgb(34, 34, 34), 2);
}

void SchematicCanvas::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted() && itemAt(event->scenePos(), QTransform()) != selectionPath) return;

    switch (event->button()) {
        case Qt::LeftButton:
            leftMousePressEvent(event);
            break;
    }
}

void SchematicCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    if (event->isAccepted() && itemAt(event->scenePos(), QTransform()) != selectionPath) return;

    switch (event->button()) {
        case Qt::LeftButton:
            leftMouseReleaseEvent(event);
            break;
    }
}

void SchematicCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
    if (event->isAccepted() && itemAt(event->scenePos(), QTransform()) != selectionPath) return;

    event->ignore();

    if (isSelecting) {
        selectionPoints.append(event->scenePos());

        auto path = QPainterPath();
        path.moveTo(selectionPoints.first());
        for (auto i = 1; i < selectionPoints.size(); i++) {
            path.lineTo(selectionPoints[i]);
        }
        path.closeSubpath();

        selectionPath->setPath(path);
        selectionPath->setVisible(true);

        auto selectItems = items(path);
        for (auto &item : selectItems) {
            auto nodeItem = dynamic_cast<NodeItem *>(item);
            if (nodeItem) {
                nodeItem->node->select(false);
            }
        }

        event->accept();
    }
}

void SchematicCanvas::keyPressEvent(QKeyEvent *event) {
    if (focusItem()) {
        QGraphicsScene::keyPressEvent(event);
    } else if (event->matches(QKeySequence::Delete)) {
        schematic->deleteSelectedItems();
    }
}

void SchematicCanvas::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    QGraphicsScene::contextMenuEvent(event);
    if (event->isAccepted()) return;

    auto scenePos = event->scenePos();

    QMenu menu;
    auto contextSearch = new QLineEdit();
    contextSearch->setPlaceholderText("Search modules...");
    auto widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(contextSearch);
    menu.addAction(widgetAction);
    menu.addSeparator();

    auto newNodeAction = menu.addAction(tr("New Node"));
    connect(newNodeAction, &QAction::triggered,
            [this, scenePos]() { newNode(scenePos); });

    menu.addSeparator();
    menu.addAction(new QAction(tr("LFO")));
    menu.addAction(new QAction(tr("Something else")));

    menu.exec(event->screenPos());
}

void SchematicCanvas::leftMousePressEvent(QGraphicsSceneMouseEvent *event) {
    isSelecting = true;
    if (!(event->modifiers() & Qt::ShiftModifier)) {
        schematic->deselectAll();
    }
    selectionPoints.append(event->scenePos());
    event->accept();
}

void SchematicCanvas::leftMouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (!isSelecting) {
        event->ignore();
        return;
    }

    isSelecting = false;
    selectionPoints.clear();
    selectionPath->setVisible(false);
    event->accept();
}

void SchematicCanvas::drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color,
                               qreal pointSize) {
    QPointF topLeft = rect.topLeft();
    topLeft.setX(std::floor(topLeft.x() / size.width()) * size.width());
    topLeft.setY(std::floor(topLeft.y() / size.height()) * size.height());

    QPointF bottomRight = rect.bottomRight();
    bottomRight.setX(std::ceil(bottomRight.x() / size.width()) * size.width());
    bottomRight.setY(std::ceil(bottomRight.y() / size.height()) * size.height());

    auto drawPen = QPen(color);
    drawPen.setWidthF(pointSize);
    painter->setPen(drawPen);

    for (auto x = topLeft.x(); x < bottomRight.x(); x += size.width()) {
        for (auto y = topLeft.y(); y < bottomRight.y(); y += size.height()) {
            painter->drawPoint((int) x + 1, (int) y + 1);
        }
    }
}
