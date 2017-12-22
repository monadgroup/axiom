#include "NodeResizer.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QCursor>
#include <QtGui/QPainter>

using namespace AxiomGui;

NodeResizer::NodeResizer(Direction dir, float marginSize) : dir(dir), marginSize(marginSize) {
    setAcceptedMouseButtons(Qt::LeftButton);

    switch (dir) {
        case TOP: case BOTTOM: setCursor(Qt::SizeVerCursor); break;
        case LEFT: case RIGHT: setCursor(Qt::SizeHorCursor); break;
        case TOP_RIGHT: case BOTTOM_LEFT: setCursor(Qt::SizeBDiagCursor); break;
        case TOP_LEFT: case BOTTOM_RIGHT: setCursor(Qt::SizeFDiagCursor); break;
    }
}

QRectF NodeResizer::boundingRect() const {
    QPointF p = QPointF(-marginSize, -marginSize);
    QSizeF s = QSizeF(m_size.width() + marginSize*2, m_size.height() + marginSize*2);

    if (dir & (TOP | BOTTOM)) {
        s.setHeight(marginSize);
    }
    if (dir & (LEFT | RIGHT)) {
        s.setWidth(marginSize);
    }
    if (dir & BOTTOM) {
        p.setY(m_size.height());
    }
    if (dir & RIGHT) {
        p.setX(m_size.width());
    }

    return {p, s};
}

void NodeResizer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setBrush(QBrush(Qt::red));
    painter->setPen(Qt::transparent);
    painter->drawRect(boundingRect());
}

void NodeResizer::setSize(QSizeF newSize) {
    if (newSize != m_size) {
        m_size = newSize;
        update();
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
        newSize.setHeight(startSize.height() + mouseDelta.y());
    }
    if (dir & RIGHT) {
        newSize.setWidth(startSize.width() + mouseDelta.x());
    }
    if (dir & TOP) {
        newPos.setY(startPos.y() + mouseDelta.y());
        newSize.setHeight(startSize.height() - mouseDelta.y());
    }
    if (dir & LEFT) {
        newPos.setX(startPos.x() + mouseDelta.x());
        newSize.setWidth(startSize.width() - mouseDelta.x());
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
