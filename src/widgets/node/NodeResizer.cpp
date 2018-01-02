#include "NodeResizer.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QCursor>
#include <QtGui/QPainter>

using namespace AxiomGui;

NodeResizer::NodeResizer(Direction dir, QSizeF minSize, float marginSize)
        : dir(dir), minSize(minSize), marginSize(marginSize) {
    setAcceptedMouseButtons(Qt::LeftButton);

    switch (dir) {
        case TOP:
        case BOTTOM:
            setCursor(Qt::SizeVerCursor);
            break;
        case LEFT:
        case RIGHT:
            setCursor(Qt::SizeHorCursor);
            break;
        case TOP_RIGHT:
        case BOTTOM_LEFT:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case TOP_LEFT:
        case BOTTOM_RIGHT:
            setCursor(Qt::SizeFDiagCursor);
            break;
    }
}

QRectF NodeResizer::boundingRect() const {
    QPointF p = QPointF(-marginSize / 2, -marginSize / 2);
    QSizeF s = QSizeF(m_size.width() + marginSize, m_size.height() + marginSize);

    auto hasV = dir & (TOP | BOTTOM);
    auto hasH = dir & (LEFT | RIGHT);

    if (hasV) {
        s.setHeight(marginSize * (hasH ? 2 : 1));
    }
    if (hasH) {
        s.setWidth(marginSize * (hasV ? 2 : 1));
    }
    if (dir & BOTTOM) {
        p.setY(m_size.height() - marginSize * (hasH ? 1.5f : 0.5f));
    }
    if (dir & RIGHT) {
        p.setX(m_size.width() - marginSize * (hasV ? 1.5f : 0.5f));
    }

    return {p, s};
}

void NodeResizer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
}

void NodeResizer::setSize(QSizeF newSize) {
    if (newSize != m_size) {
        prepareGeometryChange();
        m_size = newSize;
    }
}

void NodeResizer::setPos(QPointF newPos) {
    if (newPos != m_pos) {
        m_pos = newPos;
    }
}

void NodeResizer::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = true;
    startMousePos = event->screenPos();
    startTopLeft = m_pos;
    startBottomRight = QPointF(m_pos.x() + m_size.width(), m_pos.y() + m_size.height());
    emit startDrag(m_pos, m_size);
}

void NodeResizer::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto topLeft = m_pos;
    auto bottomRight = QPointF(m_pos.x() + m_size.width(), m_pos.y() + m_size.height());
    auto startSize = startBottomRight - startTopLeft;

    auto mouseDelta = event->screenPos() - startMousePos;

    if (dir & BOTTOM) {
        auto newHeight = qMax(startSize.y() + mouseDelta.y(), minSize.height());
        bottomRight.setY(startTopLeft.y() + newHeight);
    }
    if (dir & RIGHT) {
        auto newWidth = qMax(startSize.x() + mouseDelta.x(), minSize.width());
        bottomRight.setX(startTopLeft.x() + newWidth);
    }
    if (dir & TOP) {
        auto newHeight = qMax(startSize.y() - mouseDelta.y(), minSize.height());
        topLeft.setY(startBottomRight.y() - newHeight);
    }
    if (dir & LEFT) {
        auto newWidth = qMax(startSize.x() - mouseDelta.x(), minSize.height());
        topLeft.setX(startBottomRight.x() - newWidth);
    }

    if (topLeft != startTopLeft || bottomRight != startBottomRight) emit changed(topLeft, bottomRight);
}

void NodeResizer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    emit endDrag(m_pos, m_size);
}
