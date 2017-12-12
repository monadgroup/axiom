#pragma once
#include <QtWidgets/QGraphicsView>

namespace AxiomGui {

    class NodeViewCanvas : public QGraphicsView {
        Q_OBJECT

    public:
        static QSize gridSize;

        explicit NodeViewCanvas(QWidget *parent = nullptr);

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect) override;

        void resizeEvent(QResizeEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void wheelEvent(QWheelEvent *event) override;

    private:
        bool isDragging = false;
        QPoint centerPoint;

        float zoom = 0;
        QPointF offset = QPointF(0, 0);

        void updateMatrix();

        static void drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize);
    };

}
