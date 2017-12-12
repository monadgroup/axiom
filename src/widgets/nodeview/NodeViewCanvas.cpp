#include "NodeViewCanvas.h"

#include <cmath>
#include <iostream>
#include <QtGui/QResizeEvent>

#include "src/AxiomApplication.h"

using namespace AxiomGui;

QSize NodeViewCanvas::gridSize = QSize(50, 50);

NodeViewCanvas::NodeViewCanvas(QWidget *parent) : QGraphicsView(new QGraphicsScene(), parent) {
    scene()->setSceneRect(0, 0, width(), height());
}

void NodeViewCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
    drawGrid(painter, rect, gridSize, QColor::fromRgb(34, 34, 34), 2/std::pow(2, this->zoom));
}

void NodeViewCanvas::resizeEvent(QResizeEvent *event) {
    scene()->setSceneRect(0, 0, event->size().width(), event->size().height());
}

void NodeViewCanvas::mousePressEvent(QMouseEvent *event) {
    //AxiomApplication::main->setOverrideCursor(Qt::BlankCursor);
    isDragging = true;
    centerPoint = mapToGlobal(QPoint(width() / 2, height() / 2));
    cursor().setPos(centerPoint);
}

void NodeViewCanvas::mouseReleaseEvent(QMouseEvent *event) {
    //AxiomApplication::main->restoreOverrideCursor();
    isDragging = false;
}

void NodeViewCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (!isDragging) return;

    auto deltaPos = cursor().pos() - centerPoint;
    cursor().setPos(centerPoint);

    offset += QPointF(deltaPos.x(), deltaPos.y()) / 10.f;
    std::cout << offset.x() << "x" << offset.y() << std::endl;
    updateMatrix();
}

void NodeViewCanvas::wheelEvent(QWheelEvent *event) {
    this->zoom += event->delta() / 2000.0f;
    updateMatrix();
}

void NodeViewCanvas::updateMatrix() {
    auto visualZoom = std::pow(2, this->zoom);
    setMatrix(
        QMatrix().scale(visualZoom, visualZoom)
                .translate(offset.x(), offset.y())
    );
}

void NodeViewCanvas::drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize) {
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
