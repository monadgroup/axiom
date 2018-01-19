#include "SchematicCanvas.h"

#include <cmath>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsPathItem>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "editor/AxiomApplication.h"
#include "../node/NodeItem.h"
#include "editor/model/node/CustomNode.h"

using namespace AxiomGui;
using namespace AxiomModel;

QSize SchematicCanvas::nodeGridSize = QSize(50, 50);

QSize SchematicCanvas::controlGridSize = QSize(25, 25);

SchematicCanvas::SchematicCanvas(Schematic *schematic) : schematic(schematic) {
    // build selection
    auto selectionPen = QPen(QColor::fromRgb(52, 152, 219));
    auto selectionBrush = QBrush(QColor::fromRgb(52, 152, 219, 50));

    selectionPath = addPath(QPainterPath(), selectionPen, selectionBrush);
    selectionPath->setVisible(false);
    selectionPath->setZValue(1);

    // create items for all nodes that already exist
    for (const auto &item : schematic->items()) {
        if (auto node = dynamic_cast<Node *>(item.get())) {
            addNode(node);
        }
    }

    // connect to model
    connect(schematic, &Schematic::panChanged,
            this, &SchematicCanvas::setPan);
    connect(schematic, &Schematic::itemAdded,
            [this](AxiomModel::GridItem *item) {
                if (auto node = dynamic_cast<Node *>(item)) {
                    addNode(node);
                }
            });
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

QSize SchematicCanvas::controlRealSize(const QSize &s) {
    return {
            s.width() * SchematicCanvas::controlGridSize.width(),
            s.height() * SchematicCanvas::controlGridSize.height()
    };
}

void SchematicCanvas::setPan(QPointF pan) {
    // todo
}

void SchematicCanvas::addNode(AxiomModel::Node *node) {
    auto item = new NodeItem(node, this);
    item->setZValue(0);
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
        default:
            QGraphicsScene::mousePressEvent(event);
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
        default:
            QGraphicsScene::mouseReleaseEvent(event);
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
    if (event->matches(QKeySequence::Delete)) {
        schematic->deleteSelectedItems();
    }
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
