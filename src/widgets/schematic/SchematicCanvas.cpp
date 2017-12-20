#include "SchematicCanvas.h"

#include <cmath>
#include <iostream>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsPathItem>

#include "src/AxiomApplication.h"
#include "../node/NodeItem.h"

using namespace AxiomGui;

QSize SchematicCanvas::gridSize = QSize(50, 50);

SchematicCanvas::SchematicCanvas(QWidget *parent) : QGraphicsView(new QGraphicsScene(), parent) {
    scene()->setSceneRect(0, 0, width()*2, height()*2);

    setDragMode(QGraphicsView::RubberBandDrag);
    setRenderHint(QPainter::Antialiasing);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    auto selectionPen = QPen(QColor::fromRgb(52, 152, 219));
    auto selectionBrush = QBrush(QColor::fromRgb(52, 152, 219, 50));

    selectionPath = scene()->addPath(QPainterPath(), selectionPen, selectionBrush);
    selectionPath->setVisible(false);
    selectionPath->setZValue(100);

    scene()->addItem(new NodeItem());
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
    }
}

void SchematicCanvas::mouseReleaseEvent(QMouseEvent *event) {
    switch (event->button()) {
        case Qt::LeftButton: leftMouseReleaseEvent(event); break;
        case Qt::MiddleButton: middleMouseReleaseEvent(event); break;
    }
}

void SchematicCanvas::mouseMoveEvent(QMouseEvent *event) {
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
    isSelecting = true;
    selectionPoints.append(event->localPos());
}

void SchematicCanvas::leftMouseReleaseEvent(QMouseEvent *event) {
    if (!isSelecting) return;

    isSelecting = false;
    selectionPoints.clear();
    selectionPath->setVisible(false);
}

void SchematicCanvas::middleMousePressEvent(QMouseEvent *event) {
    isDragging = true;

    dragStart = event->localPos();
    dragOffset = QPointF(0, 0);
}

void SchematicCanvas::middleMouseReleaseEvent(QMouseEvent *event) {
    isDragging = false;
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
