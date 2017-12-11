#include "NodeViewCanvas.h"

#include <cmath>
#include <iostream>
#include <QtGui/QResizeEvent>

using namespace AxiomGui;

QSize NodeViewCanvas::gridSize = QSize(50, 50);

NodeViewCanvas::NodeViewCanvas(QWidget *parent) : QGraphicsView(new QGraphicsScene(), parent) {
    scene()->setSceneRect(0, 0, width(), height());
}

void NodeViewCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
    QPointF topLeft = rect.topLeft();
    topLeft.setX(std::floor(topLeft.x() / gridSize.width()) * gridSize.width());
    topLeft.setY(std::floor(topLeft.y() / gridSize.height()) * gridSize.height());

    QPointF bottomRight = rect.bottomRight();
    bottomRight.setX(std::ceil(bottomRight.x() / gridSize.width()) * gridSize.width());
    bottomRight.setY(std::ceil(bottomRight.y() / gridSize.height()) * gridSize.height());

    QPen drawPen(QColor::fromRgb(34, 34, 34)); // #222
    drawPen.setWidth(2);
    painter->setPen(drawPen);

    for (auto x = topLeft.x(); x < bottomRight.x(); x += gridSize.width()) {
        for (auto y = topLeft.y(); y < bottomRight.y(); y += gridSize.height()) {
            painter->drawPoint((int)x, (int)y);
        }
    }
}

void NodeViewCanvas::resizeEvent(QResizeEvent *event) {
    scene()->setSceneRect(0, 0, event->size().width(), event->size().height());
}
