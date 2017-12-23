#include "NodeResizer.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QCursor>
#include <QtGui/QPainter>

using namespace AxiomGui;

NodeResizer::NodeResizer(Direction dir, QSizeF minSize, float marginSize)
        : dir(dir), minSize(minSize), marginSize(marginSize) {
    setAcceptedMouseButtons(Qt::LeftButton);

    switch (dir) {
        case TOP: case BOTTOM: setCursor(Qt::SizeVerCursor); break;
        case LEFT: case RIGHT: setCursor(Qt::SizeHorCursor); break;
        case TOP_RIGHT: case BOTTOM_LEFT: setCursor(Qt::SizeBDiagCursor); break;
        case TOP_LEFT: case BOTTOM_RIGHT: setCursor(Qt::SizeFDiagCursor); break;
    }
}

QRectF NodeResizer::boundingRect() const {
    QPointF p = QPointF(-marginSize/2, -marginSize/2);
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
    startPos = m_pos;
    startSize = m_size;
    emit startDrag(m_pos, m_size);
}

void NodeResizer::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (!isDragging) return;

    auto newSize = m_size;
    auto newPos = m_pos;

    auto mouseDelta = event->screenPos() - startMousePos;

    if (dir & BOTTOM) {
        newSize.setHeight(qMax(startSize.height() + mouseDelta.y(), minSize.height()));
    }
    if (dir & RIGHT) {
        newSize.setWidth(qMax(startSize.width() + mouseDelta.x(), minSize.width()));
    }
    if (dir & TOP) {
        auto newHeight = qMax(startSize.height() - mouseDelta.y(), minSize.height());
        newPos.setY(startPos.y() + startSize.height() - newHeight);
        newSize.setHeight(newHeight);
    }
    if (dir & LEFT) {
        auto newWidth = qMax(startSize.width() - mouseDelta.x(), minSize.width());
        newPos.setX(startPos.x() + startSize.width() - newWidth);
        newSize.setWidth(newWidth);
    }

    if (newPos != m_pos) emit moved(newPos);
    if (newSize != m_size) emit resized(newSize);
}

void NodeResizer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    setPos(m_pos);
    setSize(m_size);
    emit endDrag(m_pos, m_size);
}
