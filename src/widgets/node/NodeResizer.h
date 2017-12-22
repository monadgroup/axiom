#pragma once
#include <QtWidgets/QGraphicsObject>

namespace AxiomGui {

    class NodeResizer : public QGraphicsObject {
        Q_OBJECT

    public:
        enum Direction {
            TOP    = 1 << 0,
            RIGHT  = 1 << 1,
            BOTTOM = 1 << 2,
            LEFT   = 1 << 3,

            TOP_RIGHT    = TOP | RIGHT,
            TOP_LEFT     = TOP | LEFT,
            BOTTOM_RIGHT = BOTTOM | RIGHT,
            BOTTOM_LEFT  = BOTTOM | LEFT
        };

        explicit NodeResizer(Direction dir, float marginSize = 5);

        QRectF boundingRect() const override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    public slots:
        void setSize(QSizeF newSize);
        void setPos(QPointF newPos);

    signals:
        void startDrag(QPointF pos, QSizeF size);
        void endDrag(QPointF pos, QSizeF size);
        void moved(QPointF pos);
        void resized(QSizeF size);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    private:
        float marginSize;
        Direction dir;
        bool isDragging;
        QPointF startMousePos;
        QPointF startPos;
        QSizeF startSize;

        QPointF m_pos;
        QSizeF m_size;
    };

}
