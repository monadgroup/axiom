#pragma once

#include <QtWidgets/QGraphicsObject>

namespace AxiomGui {

    class ItemResizer : public QGraphicsObject {
        Q_OBJECT

    public:
        enum Direction {
            TOP = 1 << 0,
            RIGHT = 1 << 1,
            BOTTOM = 1 << 2,
            LEFT = 1 << 3,

            TOP_RIGHT = TOP | RIGHT,
            TOP_LEFT = TOP | LEFT,
            BOTTOM_RIGHT = BOTTOM | RIGHT,
            BOTTOM_LEFT = BOTTOM | LEFT
        };

        explicit ItemResizer(Direction dir, QSizeF minSize, float marginSize = 5);

        QRectF boundingRect() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        bool doPaint() const { return m_doPaint; }

    public slots:

        void setSize(QSizeF newSize);

        void setPos(QPointF newPos);

        void setDoPaint(bool newDoPaint);

        void enablePainting() { setDoPaint(true); }

        void disablePainting() { setDoPaint(false); }

    signals:

        void startDrag(QPointF pos, QSizeF size);

        void endDrag(QPointF pos, QSizeF size);

        void changed(QPointF topLeft, QPointF bottomRight);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    private:
        float marginSize;
        Direction dir;
        bool isDragging = false;
        QPointF startMousePos;
        QPointF startTopLeft;
        QPointF startBottomRight;
        QSizeF minSize;

        bool m_doPaint = false;
        QPointF m_pos;
        QSizeF m_size;
    };
}
