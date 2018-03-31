#include "ItemResizer.h"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QCursor>
#include <QtGui/QPainter>

using namespace AxiomGui;

ItemResizer::ItemResizer(Direction dir, QSizeF minSize, float marginSize)
    : marginSize(marginSize), dir(dir), minSize(minSize) {
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

QRectF ItemResizer::boundingRect() const {
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

void ItemResizer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (!m_doPaint) return;

    painter->setPen(QPen(QColor(0, 0, 0), 1));
    painter->setBrush(QBrush(QColor(255, 255, 255)));

    if (dir == Direction::TOP_LEFT || dir == Direction::TOP_RIGHT || dir == Direction::BOTTOM_LEFT ||
        dir == Direction::BOTTOM_RIGHT) {
        QPointF p = QPointF(-marginSize / 2, -marginSize / 2);
        QSizeF s = QSizeF(m_size.width() + marginSize, m_size.height() + marginSize);

        auto hasV = dir & (TOP | BOTTOM);
        auto hasH = dir & (LEFT | RIGHT);

        if (hasV) {
            s.setHeight(marginSize);
        }
        if (hasH) {
            s.setWidth(marginSize);
        }
        if (dir & BOTTOM) {
            p.setY(m_size.height() - marginSize / 2.);
        }
        if (dir & RIGHT) {
            p.setX(m_size.width() - marginSize / 2.);
        }
        painter->drawRect({p, s});

    } else {
        QPointF p;
        QSizeF s;

        if (dir & Direction::LEFT) {
            p = QPointF(0, 0);
            s = QSizeF(1.5, m_size.height());
        }
        if (dir & Direction::BOTTOM) {
            p = QPointF(0, m_size.height());
            s = QSizeF(m_size.width(), 1.5);
        }
        if (dir & Direction::RIGHT) {
            p = QPointF(m_size.width(), 0);
            s = QSizeF(1.5, m_size.height());
        }
        if (dir & Direction::TOP) {
            p = QPointF(0, 0);
            s = QSizeF(m_size.width(), 1.5);
        }

        painter->drawRect({p, s});
    }
}

void ItemResizer::setSize(QSizeF newSize) {
    if (newSize != m_size) {
        prepareGeometryChange();
        m_size = newSize;
    }
}

void ItemResizer::setPos(QPointF newPos) {
    if (newPos != m_pos) {
        m_pos = newPos;
    }
}

void ItemResizer::setDoPaint(bool newDoPaint) {
    if (newDoPaint != m_doPaint) {
        m_doPaint = newDoPaint;
        update();
    }
}

void ItemResizer::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = true;
    startMousePos = event->screenPos();
    startTopLeft = m_pos;
    startBottomRight = QPointF(m_pos.x() + m_size.width(), m_pos.y() + m_size.height());
    emit startDrag(m_pos, m_size);
}

void ItemResizer::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
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

void ItemResizer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    isDragging = false;
    emit endDrag(m_pos, m_size);
}
