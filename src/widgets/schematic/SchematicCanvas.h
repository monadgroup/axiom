#pragma once
#include <QtWidgets/QGraphicsView>

namespace AxiomGui {

    class SchematicCanvas : public QGraphicsView {
        Q_OBJECT

    public:
        static QSize gridSize;

        explicit SchematicCanvas(QWidget *parent = nullptr);

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect) override;

        void resizeEvent(QResizeEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;

    private:
        bool isSelecting = false;
        bool isDragging = false;

        QVector<QPointF> selectionPoints;
        QGraphicsPathItem *selectionPath;

        QPointF dragStart;
        QPointF dragOffset;

        void leftMousePressEvent(QMouseEvent *event);
        void leftMouseReleaseEvent(QMouseEvent *event);
        void middleMousePressEvent(QMouseEvent *event);
        void middleMouseReleaseEvent(QMouseEvent *event);

        static void drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize);
    };

}
