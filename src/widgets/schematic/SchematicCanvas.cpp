#include "SchematicCanvas.h"

#include <cmath>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsPathItem>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include "src/AxiomApplication.h"
#include "../node/NodeItem.h"
#include "src/model/CustomNode.h"

using namespace AxiomGui;
using namespace AxiomModel;

QSize SchematicCanvas::gridSize = QSize(50, 50);

SchematicCanvas::SchematicCanvas(Schematic *schematic) : schematic(schematic) {
    // build selection
    auto selectionPen = QPen(QColor::fromRgb(52, 152, 219));
    auto selectionBrush = QBrush(QColor::fromRgb(52, 152, 219, 50));

    selectionPath = addPath(QPainterPath(), selectionPen, selectionBrush);
    selectionPath->setVisible(false);
    selectionPath->setZValue(100);

    // create items for all nodes that already exist
    for (const auto &item : schematic->nodes()) {
        addNode(item.get());
    }

    // connect to model
    connect(schematic, &Schematic::panChanged,
            this, &SchematicCanvas::setPan);
    connect(schematic, &Schematic::nodeAdded,
            this, &SchematicCanvas::addNode);
}

QPoint SchematicCanvas::nodeRealPos(const QPoint &p) {
    return {
            p.x() * SchematicCanvas::gridSize.width(),
            p.y() * SchematicCanvas::gridSize.height()
    };
}

QSize SchematicCanvas::nodeRealSize(const QSize &s) {
    return {
            s.width() * SchematicCanvas::gridSize.width() + 1,
            s.height() * SchematicCanvas::gridSize.height() + 1
    };
}

void SchematicCanvas::setPan(QPointF pan) {
    // todo
}

void SchematicCanvas::addNode(AxiomModel::Node *node) {
    addItem(new NodeItem(node, this));
}

void SchematicCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
    drawGrid(painter, rect, gridSize, QColor::fromRgb(34, 34, 34), 2);
}

void SchematicCanvas::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted() && itemAt(event->scenePos(), QTransform()) != selectionPath) return;

    switch (event->button()) {
        case Qt::LeftButton:
            leftMousePressEvent(event);
            break;
        case Qt::MiddleButton:
            middleMousePressEvent(event);
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
        case Qt::MiddleButton:
            middleMouseReleaseEvent(event);
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

    if (isDragging) {
        // todo
        event->accept();
    }
}

void SchematicCanvas::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::Delete)) {
        schematic->deleteSelectedNodes();
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

void SchematicCanvas::middleMousePressEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = true;

    dragStart = event->pos();
    dragOffset = QPointF(0, 0);
    event->accept();
}

void SchematicCanvas::middleMouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
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

    //QPen drawPen(QColor::fromRgb(34, 34, 34)); // #222
    auto drawPen = QPen(color);
    drawPen.setWidthF(pointSize);
    painter->setPen(drawPen);

    for (auto x = topLeft.x(); x < bottomRight.x(); x += size.width()) {
        for (auto y = topLeft.y(); y < bottomRight.y(); y += size.height()) {
            painter->drawPoint((int) x + 1, (int) y + 1);
        }
    }
}
