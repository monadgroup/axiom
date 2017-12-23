#pragma once
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

namespace AxiomModel {
    class Schematic;
    class Node;
}

namespace AxiomGui {

    class SchematicCanvas : public QGraphicsView {
        Q_OBJECT

    public:
        static QSize gridSize;

        AxiomModel::Schematic *schematic;

        explicit SchematicCanvas(AxiomModel::Schematic *schematic, QWidget *parent = nullptr);

        static QPoint nodeRealPos(const QPoint &p);
        static QSize nodeRealSize(const QSize &s);

    public slots:
        void setPan(QPointF pan);
        void addNode(AxiomModel::Node *node);
        void newNode();

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

        QMenu *contextMenu;

        void leftMousePressEvent(QMouseEvent *event);
        void leftMouseReleaseEvent(QMouseEvent *event);
        void middleMousePressEvent(QMouseEvent *event);
        void middleMouseReleaseEvent(QMouseEvent *event);

        void contextMenuEvent(QContextMenuEvent *event) override;

        static void drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize);
    };

}
