#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

namespace AxiomModel {
    class Schematic;

    class Node;
}

namespace AxiomGui {

    class SchematicCanvas : public QGraphicsScene {
    Q_OBJECT

    public:
        static QSize gridSize;

        AxiomModel::Schematic *schematic;

        explicit SchematicCanvas(AxiomModel::Schematic *schematic);

        static QPoint nodeRealPos(const QPoint &p);

        static QSize nodeRealSize(const QSize &s);

    public slots:

        void setPan(QPointF pan);

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect) override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void keyPressEvent(QKeyEvent *event) override;

    private slots:

        void addNode(AxiomModel::Node *node);

    private:
        bool isSelecting = false;
        bool isDragging = false;

        QVector<QPointF> selectionPoints;
        QGraphicsPathItem *selectionPath;

        QPointF dragStart;
        QPointF dragOffset;

        void leftMousePressEvent(QGraphicsSceneMouseEvent *event);

        void leftMouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        void middleMousePressEvent(QGraphicsSceneMouseEvent *event);

        void middleMouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        static void
        drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize);
    };

}
