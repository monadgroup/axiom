#include "SchematicCanvas.h"

#include <cmath>
#include <iostream>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsPathItem>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidgetAction>

#include "src/AxiomApplication.h"
#include "../node/NodeItem.h"

using namespace AxiomGui;
using namespace AxiomModel;

QSize SchematicCanvas::gridSize = QSize(50, 50);

SchematicCanvas::SchematicCanvas(Schematic *schematic, QWidget *parent)
        : QGraphicsView(new QGraphicsScene(), parent), schematic(schematic) {
    scene()->setSceneRect(0, 0, width()*2, height()*2);

    // set properties
    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // build selection
    auto selectionPen = QPen(QColor::fromRgb(52, 152, 219));
    auto selectionBrush = QBrush(QColor::fromRgb(52, 152, 219, 50));

    selectionPath = scene()->addPath(QPainterPath(), selectionPen, selectionBrush);
    selectionPath->setVisible(false);
    selectionPath->setZValue(100);

    std::cout << "Schematic is " << schematic << std::endl;

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
        s.width() * SchematicCanvas::gridSize.width(),
        s.height() * SchematicCanvas::gridSize.height()
    };
}

void SchematicCanvas::setPan(QPointF pan) {
    // todo
}

void SchematicCanvas::addNode(AxiomModel::Node *node) {
    scene()->addItem(new NodeItem(node, this));
}

void SchematicCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
    drawGrid(painter, rect, gridSize, QColor::fromRgb(34, 34, 34), 2);
}

void SchematicCanvas::resizeEvent(QResizeEvent *event) {
    scene()->setSceneRect(0, 0, event->size().width(), event->size().height());
}

void SchematicCanvas::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
        case Qt::LeftButton: leftMousePressEvent(event); break;
        case Qt::MiddleButton: middleMousePressEvent(event); break;
        default: QGraphicsView::mousePressEvent(event); break;
    }
}

void SchematicCanvas::mouseReleaseEvent(QMouseEvent *event) {
    switch (event->button()) {
        case Qt::LeftButton: leftMouseReleaseEvent(event); break;
        case Qt::MiddleButton: middleMouseReleaseEvent(event); break;
        default: QGraphicsView::mouseReleaseEvent(event); break;
    }
}

void SchematicCanvas::mouseMoveEvent(QMouseEvent *event) {
    QGraphicsView::mouseMoveEvent(event);

    if (isSelecting) {
        selectionPoints.append(event->localPos());

        auto path = QPainterPath();
        path.moveTo(selectionPoints.first());
        for (auto i = 1; i < selectionPoints.size(); i++) {
            path.lineTo(selectionPoints[i]);
        }
        path.closeSubpath();

        scene()->setSelectionArea(path);
        selectionPath->setPath(path);
        selectionPath->setVisible(true);
    }

    if (isDragging) {
        // todo
    }
}

void SchematicCanvas::leftMousePressEvent(QMouseEvent *event) {
    QGraphicsView::mousePressEvent(event);
    return;

    isSelecting = true;
    selectionPoints.append(event->localPos());
    event->accept();
}

void SchematicCanvas::leftMouseReleaseEvent(QMouseEvent *event) {
    QGraphicsView::mouseReleaseEvent(event);

    if (!isSelecting) return;

    isSelecting = false;
    selectionPoints.clear();
    selectionPath->setVisible(false);
    event->accept();
}

void SchematicCanvas::middleMousePressEvent(QMouseEvent *event) {
    isDragging = true;

    dragStart = event->localPos();
    dragOffset = QPointF(0, 0);
    event->accept();
}

void SchematicCanvas::middleMouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
    event->accept();
}

void SchematicCanvas::contextMenuEvent(QContextMenuEvent *event) {
    QMenu contextMenu(this);
    auto contextSearch = new QLineEdit();
    contextSearch->setPlaceholderText("Search modules...");
    auto widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(contextSearch);
    contextMenu.addAction(widgetAction);
    contextMenu.addSeparator();
    contextMenu.addAction(new QAction(tr("New Node")));
    contextMenu.addSeparator();
    contextMenu.addAction(new QAction(tr("LFO")));
    contextMenu.addAction(new QAction(tr("Something else")));

    contextMenu.exec(event->globalPos());
    event->accept();
}

void SchematicCanvas::drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize) {
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
            painter->drawPoint((int)x, (int)y);
        }
    }
}
