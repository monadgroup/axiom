#pragma once

#include <memory>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

#include "editor/model/connection/ConnectionSink.h"
#include "editor/model/connection/ConnectionWire.h"

namespace AxiomModel {
    class Schematic;

    class Node;
}

namespace AxiomGui {

    class IConnectable;

    class SchematicCanvas : public QGraphicsScene {
    Q_OBJECT

    public:
        static QSize nodeGridSize;
        static QSize controlGridSize;
        static int wireZVal;
        static int activeWireZVal;
        static int nodeZVal;
        static int activeNodeZVal;
        static int panelZVal;
        static int selectionZVal;

        AxiomModel::Schematic *schematic;

        explicit SchematicCanvas(AxiomModel::Schematic *schematic);

        static QPoint nodeRealPos(const QPoint &p);

        static QSize nodeRealSize(const QSize &s);

        static QPoint controlRealPos(const QPoint &p);

        static QPointF controlRealPos(const QPointF &p);

        static QSize controlRealSize(const QSize &s);

    public slots:

        void startConnecting(IConnectable *control);

        void updateConnecting(QPointF mousePos);

        void endConnecting(QPointF mousePos);

        void cancelConnecting();

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect) override;

        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void keyPressEvent(QKeyEvent *event) override;

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    private slots:

        void addNode(AxiomModel::Node *node);

        void newNode(QPointF scenePos);

        void addWire(AxiomModel::ConnectionWire *wire);

    private:
        bool isSelecting = false;

        QVector<QPointF> selectionPoints;
        QGraphicsPathItem *selectionPath;

        bool isConnecting = false;
        AxiomModel::ConnectionSink connectionSink;
        AxiomModel::ConnectionWire *connectionWire;

        void leftMousePressEvent(QGraphicsSceneMouseEvent *event);

        void leftMouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        static void
        drawGrid(QPainter *painter, const QRectF &rect, const QSize &size, const QColor &color, qreal pointSize);
    };

}
